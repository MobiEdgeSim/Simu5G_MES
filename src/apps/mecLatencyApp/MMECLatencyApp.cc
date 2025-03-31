//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "MMECLatencyApp.h"
#include "inet/common/Units.h"
#include "inet/networklayer/common/L3AddressResolver.h"
using namespace inet;
simsignal_t MMECLatencyApp::latency_ = registerSignal("latency");

Define_Module(MMECLatencyApp);

MMECLatencyApp::MMECLatencyApp() {
    sendPeriodicMsg = NULL;
}

MMECLatencyApp::~MMECLatencyApp() {
    cancelAndDelete(sendPeriodicMsg);
}


void MMECLatencyApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        EV << "MMECLatencyApp::initialize - True Initialize " << endl;
        sno_ = 0;
        coreNetworkDelay_ = par("coreNetworkDelay");
        int port = par("localPort");

        //retrieve parameters
        period_ = par("period");
        localPort_ = par("localPort");
        uePort_ = par("uePort");


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
        EV<<"MMECLatencyApp::initialize - try to find the upf_mec module"<<endl;
        myAddress = resolver.addressOf(getParentModule()->getSubmodule("upf_mec"));
        EV << "MMECLatencyApp::initialize - IP Address: " << myAddress.str() << endl;


        sendPeriodicMsg = new cMessage("sendPeriodicMsg");

        sendPeriodicSignal(); //send the first signal
        scheduleAt(simTime() + period_, sendPeriodicMsg);
        EV << "MMECLatencyApp::initialize - finished " << endl;
    }
}

void MMECLatencyApp::handleMessage(cMessage *msg) {
    // Sender Side
    if (msg->isSelfMessage()) { //sendPeriodicMsg

        sendPeriodicSignal();
        scheduleAt(simTime() + period_, sendPeriodicMsg);
    }

}

std::vector<cModule*> MMECLatencyApp::findAllUeModules() {
    std::vector<cModule*> ueModules;
    cModule *rootModule = getSimulation()->getSystemModule();

    for (cModule::SubmoduleIterator it(rootModule); !it.end(); ++it) {
        cModule *submodule = *it;
        if (std::string(submodule->getName()).find("ue") != std::string::npos) {
            ueModules.push_back(submodule);
        }
    }
    EV << "Found UE modules:" << endl;
    for (auto *ueModule : ueModules) {
        EV << " - " << ueModule->getFullPath() << endl;
    }
    return ueModules;
}

void MMECLatencyApp::sendPeriodicSignal() {
    std::vector<cModule*> ueModules = findAllUeModules();

    EV << "MMECLatencyApp::sendPeriodicSignal - Found " << ueModules.size() << " UE modules" << endl;

    EV << "MMECLatencyApp::sendPeriodicSignal - Current Afddress " << myAddress << endl;
    for (auto *ueModule : ueModules) {
        // get ip of ue
        inet::L3Address ueAddr = inet::L3AddressResolver().addressOf(ueModule);
        if (ueAddr.isUnspecified()) {
            EV << "MMECLatencyApp::sendPeriodicSignal - Unable to resolve IP address for module: " << ueModule->getFullName() << endl;
            continue;
        }

        EV << "For ue - " << ueModule->getFullPath() << " address is " << ueAddr << endl;

        auto periodicPkt = makeShared<PeriodicMecLatencyPacket>();
        periodicPkt->setSno(sno_++);
        periodicPkt->setReqTimestamp(simTime().dbl());
        periodicPkt->setUeAddress(ueAddr.str().c_str());
        periodicPkt->setUePort(uePort_);
        periodicPkt->setHostAddress(myAddress.str().c_str());
        periodicPkt->setHostPort(localPort_);
        int size = par("packetSize");
        periodicPkt->setChunkLength(B(size));

        inet::Packet *packet = new inet::Packet("PeriodicMecLatencyPacket");
        packet->insertAtBack(periodicPkt);

        EV << "MMECLatencyApp::sendPeriodicSignal - Sending packet to UE at address: " << ueAddr << " on port: 8000" << endl;
        socket.sendTo(packet, ueAddr, uePort_);
    }
}

