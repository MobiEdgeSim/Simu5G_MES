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

#ifndef __MECLAYENCY_UEAPP_H_
#define __MECLAYENCY_UEAPP_H_

#include "apps/mecRequestResponseApp/packets/MigrationTimer_m.h"
#include "apps/mecRequestResponseApp/packets/MecRequestResponsePacket_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "apps/mecLatencyApp/packets/MecLatencyPacket_m.h"
#include "nodes/mec/MECOrchestrator/MecOrchestrator.h"
#include "apps/mecLatencyApp/packets/PeriodicMecLatencyPacket_m.h"
using namespace omnetpp;

class UELatencyApp : public cSimpleModule {
    //inet::UdpSocket socket;
    //send probe request to mechosts to test the latency
    inet::UdpSocket probeSocket;
    simtime_t period_;
    int localPort_;
    int destPort_;
    char *sourceSymbolicAddress_;
    inet::L3Address srcAddress_;

    NRPhyUe *nrPhy_;

    unsigned int sno_;
    unsigned int bsId_;
    unsigned int appId_;

    bool enableMigration_;
    //scheduling
    cMessage *selfSender_;

    std::vector<cModule*> mecHosts;    //get from MecOrchestrator

    static simsignal_t ueReceivedLatency_;
    static simsignal_t recvResponseSno_;

public:
    ~UELatencyApp();
    UELatencyApp();
    std::map<std::string, double> latencyMap;    //save the latency from this ue to every mechost

protected:
    virtual int numInitStages() const {
        return inet::NUM_INIT_STAGES;
    }
    void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    void sendRequest();
    virtual void fetchMecHostsFromOrchestrator();
    void recvResponse(cMessage *msg);
    void handleMobileMecHost(cMessage *msg);
    //void receiverControl(cMessage *msg)

private:
    void updateMecHosts();
};

#endif
