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
#include "MECLatencyApp.h"
#include "inet/common/Units.h"
#include "inet/networklayer/common/L3AddressResolver.h"
using namespace inet;
simsignal_t MECLatencyApp::latency_ = registerSignal("latency");

Define_Module(MECLatencyApp);

void MECLatencyApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        EV << "MECLatencyApp::initialize - True Initialize " << endl;
        sno_ = 0;
        coreNetworkDelay_ = par("coreNetworkDelay");
        int port = par("localPort");
        EV << "CbrReceiver::initialize - binding to port: local:" << port << endl;
        if (port != -1) {
            socket.setOutputGate(gate("socketOut"));
            socket.bind(port);
            int tos = par("tos");
            if (tos != -1)
                socket.setTos(tos);
        }
        if (socket.getSocketId() == -1) {
            throw cRuntimeError("Failed to bind socket to port %d", port);
        }
        else {
            EV << "Successfully bound socket to port " << port << " with socket ID " << socket.getSocketId() << endl;
        }

        inet::L3AddressResolver resolver;
        myAddress = resolver.addressOf(getParentModule()->getSubmodule("upf_mec"));
        EV << "MECLatencyApp::initialize - IP Address: " << myAddress.str() << endl;

    }
}

void MECLatencyApp::handleMessage(cMessage *msg) {

    EV << "MECLatencyApp::handleMessage" << endl;
    handleRequest(msg);

}

void MECLatencyApp::handleRequest(cMessage *msg) {
    inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
    if (packet == 0)
        throw cRuntimeError("MECLatencyApp::handleRequest - FATAL! Error when casting to inet packet");
    packet->removeControlInfo();

    auto reqPkt = packet->peekAtFront<MecLatencyPacket>();

    if (reqPkt == nullptr)
        throw cRuntimeError("Packet could not be interpreted as MecLatencyPacket!");
    int sno = reqPkt->getSno();
    EV << "MECLatencyApp::handleRequest -- sno is: " << sno << endl;
    simtime_t delay = simTime() - reqPkt->getReqTimestamp();

    doubleDelay = delay.dbl();
    double delayMs = doubleDelay * 1000.0;
    // emit this delay data
    emit(latency_, (double) delayMs);
    std::string ueAddress_ = reqPkt->getUeAddress();
    inet::L3Address ueAddress = inet::L3AddressResolver().resolve(ueAddress_.c_str());
    ueLatencyMap[ueAddress.str()] = delayMs;

    EV << "I get the delay and it is :" << delayMs << "(ms) and the ue address is :" << ueAddress << endl;

    sendResponse(msg);
}

void MECLatencyApp::sendResponse(cMessage *msg) {
    inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
    if (packet == 0)
        throw cRuntimeError("MECLatencyApp::sendResponse - FATAL! Error when casting to inet packet");

    auto pkt = packet->popAtFront<MecLatencyPacket>();
    auto respPkt = inet::makeShared<MecLatencyPacket>();

    const char *ueSourceAddress = pkt->getUeAddress();
    int uePort = pkt->getUePort();

    EV << "MECLatencyApp::sendResponse - Send response for packet with number " << pkt->getSno() << " to " << ueSourceAddress << " (port " << uePort << ")" << endl;

    respPkt->setRespTimestamp(simTime().dbl());
    respPkt->setHostAddress(pkt->getHostAddress());
    respPkt->setHostPort(pkt->getHostPort());
    respPkt->setUeAddress(ueSourceAddress);
    respPkt->setUePort(uePort);
    respPkt->setUeHostLatency(doubleDelay);
    respPkt->setAppId(pkt->getAppId());
    respPkt->setBsId(pkt->getBsId());
    respPkt->setSno(pkt->getSno());
    respPkt->setReqTimestamp(pkt->getReqTimestamp());
    respPkt->setChunkLength(pkt->getChunkLength());

    // Create a new packet and insert the response packet
    auto responsePacket = new inet::Packet("MecLatencyPacket");
    responsePacket->insertAtBack(respPkt);

    //packet->insertAtBack(respPkt);
    socket.sendTo(responsePacket, inet::L3AddressResolver().resolve(ueSourceAddress), uePort);
    delete packet;
}

