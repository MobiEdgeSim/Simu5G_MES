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

#ifndef __MECLAYENCY_HOSTAPP_H_
#define __MECLAYENCY_HOSTAPP_H_

#include "apps/mecLatencyApp/packets/MecLatencyPacket_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "apps/mecLatencyApp/packets/PeriodicMecLatencyPacket_m.h"

using namespace omnetpp;

class MECLatencyApp : public cSimpleModule {
    inet::UdpSocket socket;

    double coreNetworkDelay_;

    //static simsignal_t recvRequestSno_;
    static simsignal_t latency_;

    double doubleDelay;

    unsigned int sno_;

public:

    std::map<std::string, double> ueLatencyMap; //save the latency from ue to this mechost

protected:

    virtual int numInitStages() const {
        return inet::NUM_INIT_STAGES;
    }
    void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    void handleRequest(cMessage *msg);
    void sendResponse(cMessage *msg);

    //void sendPeriodicSignal();//Add a new method to actively send signals
    inet::L3Address myAddress;

    std::vector<cModule*> findAllUeModules(); // 声明查找 UE 模块的函数

    //cMessage *sendPeriodicMsg;
    //simtime_t period;
};

#endif
