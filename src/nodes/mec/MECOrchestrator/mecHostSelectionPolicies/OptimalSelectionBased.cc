// Authors: Tianhao Zhang
#include "OptimalSelectionBased.h"
#include "nodes/mec/MECPlatformManager/MecPlatformManager.h"
#include "nodes/mec/VirtualisationInfrastructureManager/VirtualisationInfrastructureManager.h"
#include "scheduler/MetaheuristicScheduler.h"
#include "veins_inet/VeinsInetMobility.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/applications/pingapp/PingApp.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "omnetpp.h"
#include "apps/mecLatencyApp/UELatencyApp.h"
#include "apps/mecLatencyApp/MECLatencyApp.h"
#include "omnetpp/cmodule.h"
#include <random>

cModule* OptimalSelectionBased::findBestMecHost(const ApplicationDescriptor &appDesc) {
    std::string algorithm = mecOrchestrator_->algorithmName;
    // --- set app information and mechost information
    setParameters(appDesc);

    // --- MetaheuristicScheduler to get solution---
    MetaheuristicScheduler scheduler(appInfo, mecHostInfos, algorithm);
    EV << "OptimalSelectionBased::findBestMecHost - calling for JAVA algorithms to get the best mecHost" << endl;
    cModule *bestHost;
    std::string bestHostName = scheduler.findBestHost();
    if (bestHostName == "none") {
        throw("!!!!!!!!!!!!!!!!!!!!!!FLAG");
    }
    bool found = false;
    for (auto mecHost : mecOrchestrator_->mecHosts) {
        if (bestHostName.compare(mecHost->getFullName())) {
            bestHost = mecHost;
            found = true;
            break;
        }
    }

    if (found) {
        EV << "OptimalSelectionBased::findBestMecHost - Best MEC host: " << (bestHost ? bestHost->getFullName() : "None") << endl;
        //update the mec host resource
        auto *vim = check_and_cast<VirtualisationInfrastructureManager*>(bestHost->getSubmodule("vim"));
        if (vim) {
            vim->allocateResources(appInfo.ram, appInfo.disk, appInfo.cpu);
        }
    }
    else {
        EV << "OptimalSelectionBased::findBestMecHost - no MEC host found" << endl;

    }

    return bestHost;
}

/*
 * collect the app info and mec info
 */
void OptimalSelectionBased::setParameters(const ApplicationDescriptor &appDesc) {
    EV << "OptimalSelectionBased::setParameters..." << endl;

    mecHostInfos.clear();
    mecHostAddresses.clear();

    // ---APP information collection---
    EV << "OptimalSelectionBased::setParameters - Collecting information from ApplicationDescriptor" << endl;
    appInfo = collectAppInfo(appDesc);

    // ---MEC Hosts information collection---
    for (auto mecHost : mecOrchestrator_->mecHosts) {
        EV << "OptimalSelectionBased::setParameters - Collecting information from MEC host [" << mecHost->getFullName() << "], size of mecHost " << mecOrchestrator_->mecHosts.size() << endl;

        // collect mec information
        MecHostInfo info_ = collectMecHostInfo(mecHost);
        mecHostInfos.push_back(info_);
//        EV<<"Test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
//        EV<<"available RAM is "<<info.availableRam<<endl;
//        EV<<"available Disk is "<<info.availableDisk<<endl;

        mecHostAddresses.push_back(inet::L3AddressResolver().resolve(info_.ipAddress.c_str()));
    }

}

OptimalSelectionBased::AppDescriptorInfo OptimalSelectionBased::collectAppInfo(const ApplicationDescriptor &appDesc) {

    appInfo.name = appDesc.getAppName();
    appInfo.ueIpAddress = appDesc.getUeIpAddress();
    ResourceDescriptor resources = appDesc.getVirtualResources();

    //TODO the resource is fake currently
    int minRam = 2;
    int maxRam = 16;
    int minDisk = 20;
    int maxDisk = 160;
    int minCpu = 10;
    int maxCpu = 160;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> ramDist(minRam, maxRam);
    std::uniform_int_distribution<> diskDist(minDisk, maxDisk);
    std::uniform_int_distribution<> cpuDist(minCpu, maxCpu);

    appInfo.ram = ramDist(gen);
    appInfo.disk = diskDist(gen);
    appInfo.cpu = cpuDist(gen);

//    appInfo.ram = resources.ram;
//    appInfo.disk = resources.disk;
//    appInfo.cpu = resources.cpu;
    appInfo.requiredServices = appDesc.getAppServicesRequired();

    // Resolve the UE module from the IP address
    inet::L3Address ueAddress = inet::L3AddressResolver().resolve(appInfo.ueIpAddress.c_str());
    cModule *ueModule = inet::L3AddressResolver().findHostWithAddress(ueAddress);
    if (ueModule) {
        cModule *mobilityModule = ueModule->getSubmodule("mobility");
        if (mobilityModule) {
            veins::VeinsInetMobility *mobility = check_and_cast<veins::VeinsInetMobility*>(mobilityModule);
            inet::Coord lastPosition = mobility->getCurrentPosition();
            appInfo.latitude = std::round(lastPosition.x * 1000.0) / 1000.0;
            appInfo.longitude = std::round(lastPosition.y * 1000.0) / 1000.0;
            //std::cout<<"appInfo.latitude"<<appInfo.latitude<<std::endl;
            //std::cout<<"appInfo.longitude"<<appInfo.longitude<<std::endl;
        }
        else {
            EV_ERROR << "Error: Mobility module not found in UE module " << ueModule->getFullName() << "\n";
            appInfo.latitude = 0.0;
            appInfo.longitude = 0.0;
        }
    }
    else {
        EV_ERROR << "Error: UE module not found for IP address " << appInfo.ueIpAddress << "\n";
        appInfo.latitude = 0.0;
        appInfo.longitude = 0.0;
    }

    return appInfo;
}

OptimalSelectionBased::MecHostInfo OptimalSelectionBased::collectMecHostInfo(cModule *mecHost) {
    EV << "OptimalSelectionBased::collectMecHostInfo" << endl;
    MecHostInfo mechostInfo;
    mechostInfo.name = mecHost->getFullName();

    // get MEC hosts IP address
    cModule *upfModule = mecHost->getSubmodule("upf_mec");
    inet::IInterfaceTable *ift = check_and_cast<inet::IInterfaceTable*>(upfModule->getSubmodule("interfaceTable"));

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        inet::NetworkInterface *iface = ift->getInterface(i);
        if (iface->isLoopback())
            continue;

        inet::Ipv4Address ipv4Address = iface->getProtocolData<inet::Ipv4InterfaceData>()->getIPAddress();
        if (!ipv4Address.isUnspecified()) {
            mechostInfo.ipAddress = ipv4Address.str();
            break;
        }
    }

    // get VIM source
    auto *vim = check_and_cast<VirtualisationInfrastructureManager*>(mecHost->getSubmodule("vim"));
    if (vim) {
        ResourceDescriptor availResource = vim->getAvailableResources();

        mechostInfo.availableRam = availResource.ram;
        mechostInfo.availableDisk = availResource.disk;
        mechostInfo.availableCpu = availResource.cpu;

        ResourceDescriptor maxResource = vim->getMaxResources();
        mechostInfo.ram = maxResource.ram;
        mechostInfo.disk = maxResource.disk;
        mechostInfo.cpu = maxResource.cpu;
    }

    // mec services
    auto *mecpm = check_and_cast<MecPlatformManager*>(mecHost->getSubmodule("mecPlatformManager"));
    if (mecpm) {
        const std::vector<ServiceInfo> *mecServices = mecpm->getAvailableMecServices();
        if (mecServices != nullptr) {
            for (const auto &serviceInfo : *mecServices) {
                mechostInfo.availableServices.push_back(serviceInfo.getName());
            }
        }
    }

    //  MEC location
    if (mecHost->isVector()) {
        cModule *mobilityModule = mecHost->getSubmodule("mobility");
        if (mobilityModule) {
            veins::VeinsInetMobility *mobility = check_and_cast<veins::VeinsInetMobility*>(mobilityModule);
            inet::Coord lastPosition = mobility->getCurrentPosition();
//            mechostInfo.latitude = lastPosition.x;
//            mechostInfo.longitude = lastPosition.y;
            mechostInfo.latitude = std::round(lastPosition.x * 1000.0) / 1000.0;
            mechostInfo.longitude = std::round(lastPosition.y * 1000.0) / 1000.0;
        }
    }
    else {
        mechostInfo.latitude = mecHost->par("latitude").doubleValue();
        mechostInfo.longitude = mecHost->par("longitude").doubleValue();
    }

    //MEC Latency --moved to collectLatencies
    //latency information collection
    collectLatencies(mecHost, mechostInfo);

    return mechostInfo;
}

/*
 * The latencies were saved in different modules
 * -ue(for mobile mechost)
 * -mechost(for static mechost)
 *
 */
void OptimalSelectionBased::collectLatencies(cModule *mecHost, MecHostInfo &mechostInfo) {
    EV << "collect from mechost: " << mecHost->getName() << endl;
    if (strncmp(mecHost->getName(), "mecHost", strlen("mecHost")) == 0) {
        cModule *independentMecAppModule = mecHost->getSubmodule("independentMecApp", 0);
        if (independentMecAppModule == nullptr) {
            EV_ERROR << "Error: Module 'MultiMec_v2.mecHost1.independentMecApp[0]' not found.\n";
        }
        EV << "independentMecAppModule fullpath:" << independentMecAppModule->getFullPath() << endl;

        auto *mecLatencyHostApp = check_and_cast<MECLatencyApp*>(independentMecAppModule);

        if (mecLatencyHostApp) {
            std::string ueAddress = appInfo.ueIpAddress;
            std::map<std::string, double> ueLatencyMap = mecLatencyHostApp->ueLatencyMap;

            //print the map
            EV << "Complete ueLatencyMap contents:" << endl;
            for (const auto &entry : ueLatencyMap) {
                EV << "UE Address: " << entry.first << ", Latency: " << entry.second << endl;
            }
            EV << "looking for latency from this ue: " << ueAddress << endl;

            if (ueLatencyMap.find(ueAddress) != ueLatencyMap.end()) {
                mechostInfo.latency = ueLatencyMap[ueAddress];
                //mechostInfo.latency = std::round(ueLatencyMap[ueAddress] * 1000.0) / 1000.0;
            }
            else {
                mechostInfo.latency = 1e6;
                //if the mechost just appeared the latency can't get timely
                //throw cRuntimeError("OptimalSelectionBased::collectLatencies  --Can't find the mecLatencyHostApp in this mecHost!");
            }
        }

    }
    else {                //for mmecHost

        //cModule *ueModule = getModuleByPath(appInfo.ueIpAddress.c_str());
        inet::L3Address ueAddress = inet::L3AddressResolver().resolve(appInfo.ueIpAddress.c_str());
        cModule *ueModule = inet::L3AddressResolver().findHostWithAddress(ueAddress);

        UELatencyApp *ueLatencyApp = check_and_cast<UELatencyApp*>(ueModule->getSubmodule("app", 2));
        //UELatencyApp *ueLatencyApp = mecHost->getSubmodule("ueLatencyApp");
        std::map<std::string, double> ueLatencyMap;
        if (ueLatencyApp) {
            ueLatencyMap = ueLatencyApp->latencyMap;
            //<mechost, latency>
        }
        else {
            EV_ERROR << "Error: 'ueLatencyApp' module not found in UE module.\n";
        }

        std::string mechostAddress = mechostInfo.ipAddress;
        EV << "mechostAddress:" << mechostAddress << endl;

        //print the map
        EV << "Complete ueLatencyMap contents:" << endl;
        for (const auto &entry : ueLatencyMap) {
            EV << "MEC Address: " << entry.first << ", Latency: " << entry.second << endl;
        }
        EV << "looking for latency from this mobile mecHost: " << mechostAddress << endl;

        if (ueLatencyMap.find(mechostAddress) != ueLatencyMap.end()) {
            //mechostInfo.latency = ueLatencyMap[mechostAddress];
            mechostInfo.latency = std::round(ueLatencyMap[mechostAddress] * 1000.0) / 1000.0;
            EV << "Found latency for mecHost [" << mechostAddress << "] : " << mechostInfo.latency << endl;
        }
        else {
            mechostInfo.latency = 1e6;
            //if the mechost just appeared the latency can't get timely
            //throw cRuntimeError("OptimalSelectionBased::collectLatencies  --Can't find the mecLatencyHostApp in this MOBILE mecHost!");
        }
    }

}

