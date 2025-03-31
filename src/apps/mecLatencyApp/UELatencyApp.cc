//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "UELatencyApp.h"

#include "inet/networklayer/common/NetworkInterface.h"

//simsignal_t MecRequestApp::requestSize_ = registerSignal("requestSize");
//simsignal_t MecRequestApp::requestRTT_ = registerSignal("requestRTT");
//simsignal_t MecRequestApp::recvResponseSno_ = registerSignal("recvResponseSno");
simsignal_t UELatencyApp::ueReceivedLatency_ = registerSignal("ueReceivedLatency");
simsignal_t UELatencyApp::recvResponseSno_ = registerSignal("recvResponseSno");

Define_Module(UELatencyApp);

UELatencyApp::UELatencyApp() {
    selfSender_ = NULL;
}

UELatencyApp::~UELatencyApp() {
    cancelAndDelete(selfSender_);
}

void UELatencyApp::initialize(int stage) {
    EV << "UELatencyApp::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieve parameters
    period_ = par("period");
    localPort_ = par("localPort");
    destPort_ = par("destPort");
    sourceSymbolicAddress_ = (char*) getParentModule()->getFullName();

    probeSocket.setOutputGate(gate("socketOut"));
    probeSocket.bind(localPort_);

    int tos = par("tos");
    if (tos != -1)
        probeSocket.setTos(tos);

    if (probeSocket.getSocketId() == -1) {
        throw cRuntimeError("Failed to bind socket to port %d", localPort_);
    }
    else {
        EV << "Successfully bound socket to port " << localPort_ << " with socket ID " << probeSocket.getSocketId() << endl;
    }
    sno_ = 0;
    nrPhy_ = check_and_cast<NRPhyUe*>(getParentModule()->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
    bsId_ = nrPhy_->getMasterId();
    appId_ = par("appId");

    enableMigration_ = par("enableMigration").boolValue();

    //initializing the auto-scheduling messages
    selfSender_ = new cMessage("selfSender");

    //starting UELatencyApp
    simtime_t startTime = par("startTime");
    EV << "UELatencyApp::initialize - sending first packet in " << startTime << " seconds " << endl;
    scheduleAt(simTime() + startTime, selfSender_);
}

void UELatencyApp::handleMessage(cMessage *msg) {
    EV << "UELatencyApp::handleMessage " << msg->getName() << endl;
    // Sender Side
    if (msg->isSelfMessage()) {
        if (!strcmp(msg->getName(), "selfSender")) {
            EV << "UELatencyApp::handleMessage---selfmessage ---sendRequest" << endl;
            // update BS ID
            if (enableMigration_ && bsId_ != nrPhy_->getMasterId()) {
                MigrationTimer *migrationTimer = new MigrationTimer("migrationTimer");
                migrationTimer->setOldAppId(bsId_);
                migrationTimer->setNewAppId(nrPhy_->getMasterId());

                double migrationTime = uniform(20, 30);
                scheduleAt(simTime() + migrationTime, migrationTimer);
            }

            bsId_ = nrPhy_->getMasterId();
            sendRequest();

            //rescheduling
            scheduleAt(simTime() + period_, selfSender_);
        }
        else if (!strcmp(msg->getName(), "migrationTimer")) {
            MigrationTimer *migrationTimer = check_and_cast<MigrationTimer*>(msg);
            appId_ = migrationTimer->getNewAppId();
            delete migrationTimer;
        }
        else
            throw cRuntimeError("UELatencyApp::handleMessage - \tWARNING: Unrecognized self message");
    }
    // Receiver Side
    else {
        if (strcmp(msg->getName(), "MecLatencyPacket") == 0) {
            recvResponse(msg);

        }
        else if (strcmp(msg->getName(), "PeriodicMecLatencyPacket") == 0) {

            handleMobileMecHost(msg);
            //throw cRuntimeError("for test only get once");
        }
        else {
            EV << "Unhandled message type: " << msg->getName() << endl;
            throw cRuntimeError("Unhandled message type in UELatencyApp::handleMessage");
            //delete msg;
        }
    }
}

void UELatencyApp::finish() {
// ensuring there is no selfStop_ scheduled!
    if (selfSender_->isScheduled())
        cancelEvent(selfSender_);
}
/*
 * -----------------------------------------------Sender Side------------------------------------------
 */
void UELatencyApp::sendRequest() {
    fetchMecHostsFromOrchestrator();
    for (auto mechost : mecHosts) {
//const char* fullPath = mechost->getFullPath();
        cModule *virtualisationInfrastructure = mechost->getSubmodule("virtualisationInfrastructure");
        std::string fullPath = virtualisationInfrastructure->getFullPath();
        const char *cFullPath = fullPath.c_str();
        EV << "mecHost full path is: " << cFullPath << endl;

        inet::L3Address mecAddress_ = inet::L3AddressResolver().resolve(cFullPath);

        EV << "UELatencyApp::sendRequest - Attempting to send packet to " << mecAddress_.str() << " on port " << destPort_ << endl;

        int size = par("packetSize"); //

        inet::Packet *packet = new inet::Packet("MecLatencyPacket");
        auto reqPkt = makeShared<MecLatencyPacket>();
        reqPkt->setSno(sno_++);
        reqPkt->setAppId(appId_);
        reqPkt->setBsId(bsId_);
        reqPkt->setReqTimestamp(simTime().dbl());

        reqPkt->setChunkLength(B(size));
        reqPkt->setHostAddress(mecAddress_.str().c_str());
        reqPkt->setHostPort(destPort_);
        reqPkt->setUeHostLatency(0);
        reqPkt->setUeAddress(sourceSymbolicAddress_);
        reqPkt->setUePort(localPort_);

        packet->insertAtBack(reqPkt);
        probeSocket.sendTo(packet, mecAddress_, destPort_);

        EV << "UELatencyApp::sendRequest - Sending request number " << sno_ - 1 << " to " << mecAddress_.str() << endl;
    }
}

void UELatencyApp::fetchMecHostsFromOrchestrator() {
    cModule *network = getParentModule()->getParentModule();
    if (!network) {
        EV << "Error: Network module not found." << endl;
        return;
    }
    MecOrchestrator *orchestrator = check_and_cast<MecOrchestrator*>(network->getSubmodule("mecOrchestrator"));

    if (orchestrator != nullptr) {
        mecHosts = orchestrator->getMecHosts();
        EV << "Fetched " << mecHosts.size() << " MEC hosts from MecOrchestrator." << endl;
    }
    else {
        EV << "Error: MecOrchestrator module not found." << endl;
    }
}

/*
 * ---------------------------------------------Receiver Side------------------------------------------
 */

void UELatencyApp::recvResponse(cMessage *msg) {
    EV << "UELatencyApp::recvResponse" << endl;
    inet::Packet *packet = dynamic_cast<inet::Packet*>(msg);
    if (packet == 0) {
        inet::Indication *indication = dynamic_cast<inet::Indication*>(msg);
        if (indication) {
            cModule *senderModule = msg->getSenderModule();
            if (senderModule) {
                EV_WARN << "Received an inet::Indication from module: " << senderModule->getFullPath() << " (Module ID: " << senderModule->getId() << ")" << endl;
                EV_WARN << "Indication type: " << indication->getClassName() << endl;
                EV_WARN << "Indication details: " << indication->str() << endl;
            }

            else {
                EV_WARN << "Received an inet::Indication, but sender module is unknown." << endl;
            }

        }
        else {
            EV_ERROR << "Unexpected message type: " << msg->getName() << endl;
        }
        return;
    }
    packet->removeControlInfo();

    auto respPkt = packet->peekAtFront<MecLatencyPacket>();

    const char *mecHostAddress = respPkt->getHostAddress();
    int mecHostPort = respPkt->getHostPort();

    unsigned int sno = respPkt->getSno();
    double reqTimestamp = respPkt->getReqTimestamp();
    double ueHostLatency = respPkt->getUeHostLatency();

//double rtt = simTime().dbl() - reqTimestamp;

    EV << "UELatencyApp::recvResponse - sno[" << sno << "] latency is: " << ueHostLatency << endl;

    latencyMap[mecHostAddress] = ueHostLatency;
    EV << "latencyMap update - mecHostPortAddress: " << mecHostAddress << " latency is: " << ueHostLatency << endl;
// emit statistics
    emit(ueReceivedLatency_, ueHostLatency);
    emit(recvResponseSno_, (long) sno);
    delete msg;
}

void UELatencyApp::handleMobileMecHost(cMessage *msg) {

    EV << "UELatencyApp::handleMobileMecHost" << endl;
    inet::Packet *packet = dynamic_cast<inet::Packet*>(msg);
    packet->removeControlInfo();

    auto periodPkt = packet->popAtFront<PeriodicMecLatencyPacket>();
    if (periodPkt == nullptr) {
        EV_ERROR << "Failed to interpret packet as PeriodicMecLatencyPacket!" << endl;
        delete msg;
        return;
    }

    const char *mecHostAddress = periodPkt->getHostAddress();

    simtime_t delay = simTime() - periodPkt->getReqTimestamp();
    double doubleDelay = delay.dbl();
    double delayMs = doubleDelay * 1000.0;
    latencyMap[mecHostAddress] = delayMs;
    EV << "!!mobile MECHost :: latencyMap update - mecHostPortAddress: " << mecHostAddress << " latency is: " << delayMs <<"(ms)"<< endl;

    EV << "Current contents of latencyMap:" << endl;
    for (const auto &entry : latencyMap) {
        EV << "HostAddress: " << entry.first << ", Latency: " << entry.second << endl;
    }
    delete msg;
}
