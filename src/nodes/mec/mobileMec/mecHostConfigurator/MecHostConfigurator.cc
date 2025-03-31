#include "MecHostConfigurator.h"

Define_Module(MecHostConfigurator);

MecHostConfigurator::MecHostConfigurator()
{
    // 构造函数
}

MecHostConfigurator::~MecHostConfigurator()
{
    // 析构函数
}

void MecHostConfigurator::initialize(int stage)
{
    if (stage == inet::INITSTAGE_LOCAL) {
        omnetpp::cModule *upf = getParentModule()->getSubmodule("upf");
        omnetpp::cModule *vi = getParentModule()->getSubmodule("virtualisationInfrastructure");
        if (!upf || !vi) {
            throw omnetpp::cRuntimeError("Submodules 'upf' or 'virtualisationInfrastructure' not found");
        }
    }
    else if (stage == inet::INITSTAGE_NETWORK_CONFIGURATION) {
        omnetpp::cModule *upf = getParentModule()->getSubmodule("upf");
        omnetpp::cModule *vi = getParentModule()->getSubmodule("virtualisationInfrastructure");
        if (!upf || !vi) {
            throw omnetpp::cRuntimeError("Submodules 'upf' or 'virtualisationInfrastructure' not found");
        }

        // 配置内部路由
        configureInternalRoutes(upf, vi);
    }
}

void MecHostConfigurator::configureInternalRoutes(omnetpp::cModule *upf, omnetpp::cModule *vi)
{
    auto *upfIft = inet::L3AddressResolver().findInterfaceTableOf(upf);
    auto *viIft = inet::L3AddressResolver().findInterfaceTableOf(vi);

    inet::NetworkInterface *upfInterface = nullptr;
    for (int i = 0; i < upfIft->getNumInterfaces(); i++) {
        if (strcmp(upfIft->getInterface(i)->getFullName(), "eth0") == 0) {
            upfInterface = upfIft->getInterface(i);
            break;
        }
    }
    if (!upfInterface) {
        throw omnetpp::cRuntimeError("Interface 'eth0' not found in 'upf'");
    }
    auto *upfIpv4Data = upfInterface->getProtocolData<inet::Ipv4InterfaceData>();


    auto upfAddress = upfIpv4Data->getIPAddress();

    inet::NetworkInterface *viInterface = nullptr;
    for (int i = 0; i < viIft->getNumInterfaces(); i++) {
        if (strcmp(viIft->getInterface(i)->getFullName(), "eth0") == 0) {
            viInterface = viIft->getInterface(i);
            break;
        }
    }
    if (!viInterface) {
        throw omnetpp::cRuntimeError("Interface 'eth0' not found in 'virtualisationInfrastructure'");
    }
    auto *viIpv4Data = viInterface->getProtocolData<inet::Ipv4InterfaceData>();
    auto viAddress = viIpv4Data->getIPAddress();

    auto *upfRt = inet::L3AddressResolver().findIpv4RoutingTableOf(upf);
    auto *viRt = inet::L3AddressResolver().findIpv4RoutingTableOf(vi);

    addStaticRoute(upfRt, viAddress, upfInterface);
    addStaticRoute(viRt, upfAddress, viInterface);
}

void MecHostConfigurator::addStaticRoute(inet::IIpv4RoutingTable *rt, inet::Ipv4Address dest, inet::NetworkInterface *iface)
{
    auto *route = new inet::Ipv4Route();
    route->setDestination(dest);
    route->setNetmask(inet::Ipv4Address::ALLONES_ADDRESS);
    route->setGateway(inet::Ipv4Address::UNSPECIFIED_ADDRESS);
    route->setInterface(iface);
    route->setSourceType(inet::IRoute::MANUAL);
    rt->addRoute(route);
}
