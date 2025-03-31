#ifndef METAHEURISTIC_SCHEDULER_H
#define METAHEURISTIC_SCHEDULER_H

#include <jni.h>
#include <vector>
#include <string>
#include "nodes/mec/MECOrchestrator/mecHostSelectionPolicies/OptimalSelectionBased.h"
#include "JVMManager.h"

class MetaheuristicScheduler {
public:
    MetaheuristicScheduler(const OptimalSelectionBased::AppDescriptorInfo &appInfo,
                           const std::vector<OptimalSelectionBased::MecHostInfo> &mecHostInfos,
                           const std::string &algorithmName);
    ~MetaheuristicScheduler();
    std::string findBestHost();
    std::string algorithmName;

private:
    JNIEnv* env;
    jobject scheduler;
    jclass schedulerClass;
    std::string bestHostName;
};

#endif // METAHEURISTIC_SCHEDULER_H
