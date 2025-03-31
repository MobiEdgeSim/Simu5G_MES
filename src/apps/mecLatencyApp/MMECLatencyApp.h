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

#ifndef APPS_MECLATENCYAPP_MMECLATENCYAPP_H_
#define APPS_MECLATENCYAPP_MMECLATENCYAPP_H_

#include "apps/mecLatencyApp/packets/MecLatencyPacket_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "apps/mecLatencyApp/packets/PeriodicMecLatencyPacket_m.h"

using namespace omnetpp;

class MMECLatencyApp : public cSimpleModule
{
    inet::UdpSocket socket;

    //
    simtime_t period_;
    int localPort_;
    int uePort_;
    char *sourceSymbolicAddress_;
    inet::L3Address srcAddress_;

    unsigned int bsId_;
    unsigned int appId_;

    bool enableMigration_;
//

    double coreNetworkDelay_;

    //static simsignal_t recvRequestSno_;
    static simsignal_t latency_;

    double doubleDelay;

    unsigned int sno_;

public:

    std::map<std::string, double> ueLatencyMap; //save the latency from ue to this mechost
    MMECLatencyApp();
    ~MMECLatencyApp();
protected:

    virtual int numInitStages() const
    {
        return inet::NUM_INIT_STAGES;
    }
    void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    void sendPeriodicSignal(); //Add a new method to actively send signals
    inet::L3Address myAddress;

    std::vector<cModule*> findAllUeModules();

    cMessage *sendPeriodicMsg;
    simtime_t period;
};

#endif
