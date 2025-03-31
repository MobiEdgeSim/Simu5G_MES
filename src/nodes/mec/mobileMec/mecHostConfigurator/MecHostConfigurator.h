#ifndef MECHOSTCONFIGURATOR_H_
#define MECHOSTCONFIGURATOR_H_

#include <omnetpp.h>
#include <omnetpp/cmodule.h>
#include <omnetpp/cobject.h>
#include <omnetpp/cexception.h>
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/common/InitStages.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"


class MecHostConfigurator : public omnetpp::cSimpleModule
{
protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }

    void configureInternalRoutes(omnetpp::cModule *upf, omnetpp::cModule *vi);
    void addStaticRoute(inet::IIpv4RoutingTable *rt, inet::Ipv4Address dest, inet::NetworkInterface *iface);

public:
    MecHostConfigurator();
    virtual ~MecHostConfigurator();
};

#endif /* MECHOSTCONFIGURATOR_H_ */
