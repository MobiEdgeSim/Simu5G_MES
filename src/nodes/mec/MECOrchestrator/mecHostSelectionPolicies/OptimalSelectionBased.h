#ifndef OPTIMAL_SELECTION_BASED_H
#define OPTIMAL_SELECTION_BASED_H

#include "nodes/mec/MECOrchestrator/mecHostSelectionPolicies/SelectionPolicyBase.h"
#include "scheduler/JVMManager.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/applications/pingapp/PingApp.h"
#include <map>
#include <set>

class OptimalSelectionBased : public SelectionPolicyBase {
public:
    struct AppDescriptorInfo {
        std::string name;
        std::string ueIpAddress;
        double ram;
        double disk;
        double cpu;
        std::vector<std::string> requiredServices;
        double latitude;
        double longitude;
    };

    struct MecHostInfo {
        std::string name;
        std::string ipAddress;
        double availableRam;
        double availableDisk;
        double availableCpu;
        std::vector<std::string> availableServices;
        double latitude;
        double longitude;
        double latency;
        double ram;
        double disk;
        double cpu;
    };
    std::vector<MecHostInfo> mecHostInfos;
    AppDescriptorInfo appInfo;
    std::vector<inet::L3Address> mecHostAddresses;//mechost ipaddress list

public:
    virtual cModule* findBestMecHost(const ApplicationDescriptor &appDesc) override;
    void setParameters(const ApplicationDescriptor &appDesc);

private:
    inet::PingApp *pingApp = nullptr;

public:
    OptimalSelectionBased(MecOrchestrator *mecOrchestrator) :
            SelectionPolicyBase(mecOrchestrator) {
    }
    virtual ~OptimalSelectionBased() {
    }

    void finish() {
        JVMManager::destroyJVM();
    }


    AppDescriptorInfo collectAppInfo(const ApplicationDescriptor &appDesc);
    MecHostInfo collectMecHostInfo(cModule *mecHost);
    void collectLatencies(cModule *mecHost, MecHostInfo &info);
    //cModule *bestHost;

};

#endif // OPTIMAL_SELECTION_BASED_H
