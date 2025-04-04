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

#include "world/radio/ChannelAccess.h"
#include <inet/mobility/contract/IMobility.h>
#include <inet/common/ModuleAccess.h>
#include "inet/common/InitStages.h"

using namespace omnetpp;

// this is needed to ensure the correct ordering among initialization stages
// this should be fixed directly in INET
namespace inet {
Define_InitStage_Dependency(PHYSICAL_LAYER, SINGLE_MOBILITY);
}

static int parseInt(const char *s, int defaultValue) {
    if (!s || !*s)
        return defaultValue;

    char *endptr;
    int value = strtol(s, &endptr, 10);
    return *endptr == '\0' ? value : defaultValue;
}

// the destructor unregister the radio module
ChannelAccess::~ChannelAccess() {
    if (cc && myRadioRef) {
        // check if channel control exist
        IChannelControl *cc = dynamic_cast<IChannelControl*>(getSimulation()->findModuleByPath("channelControl"));
        if (cc)
            cc->unregisterRadio(myRadioRef);
        myRadioRef = nullptr;
    }
}
/**
 * Upon initialization ChannelAccess registers the nic parent module
 * to have all its connections handled by ChannelControl
 *
 *
 * tianhao changed due to the position problem of nrphy.
 * The mobile mecHost cannot get correct position to test the SINR to find the best eNodeg.
 *
 */
void ChannelAccess::initialize(int stage) {
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        cc = getChannelControl();
        hostModule = inet::findContainingNode(this);
        myRadioRef = nullptr;

        positionUpdateArrived = false;
        EV << "!!!!!!hostModule" << endl;
        EV << hostModule->getFullName() << endl;
        // subscribe to the correct mobility module
        // check if hostModule is VirtualisationInfrastructure or not
        if (strcmp(hostModule->getFullName(), "virtualisationInfrastructure") == 0) {

            cModule *mechost = getParentModule()->getParentModule()->getParentModule();  // get MECHost
            if (mechost->findSubmodule("mobility") != -1) {
                mechost->subscribe(inet::IMobility::mobilityStateChangedSignal, this);  // subscribe mobility signal through mobile mecHost
            }
            else {
                EV_WARN << "No mobility module found in MECHost: " << mechost->getFullPath() << endl;
            }
        }
        else if (hostModule->findSubmodule("mobility") != -1) {
            // register to get a notification when position changes
            hostModule->subscribe(inet::IMobility::mobilityStateChangedSignal, this);
        }
    }
    else if (stage == inet::INITSTAGE_SINGLE_MOBILITY) {
        EV<<"ChannelAccess::initialize - INITSTAGE_SINGLE_MOBILITY"<<endl;
        if (!positionUpdateArrived && hostModule->isSubscribed(inet::IMobility::mobilityStateChangedSignal, this)) {
            if (strcmp(hostModule->getFullName(), "virtualisationInfrastructure") == 0) {
                EV<<"Mobile MECHost!!"<<endl;
                cModule *mechost = getParentModule()->getParentModule()->getParentModule();  // get MECHost
                cModule *mobilityModule = mechost->getSubmodule("mobility");

                EV<<"Get mobility module:"<< mobilityModule->getFullName()<<endl;
                inet::IMobility *mobility = check_and_cast<inet::IMobility*>(mobilityModule);
                radioPos = mobility->getCurrentPosition();
                const char *s = hostModule->getParentModule()->getDisplayString().getTagArg("p", 2);
                if (s && *s)
                                error("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                                        " (3rd argument of 'p' tag)"
                                        " from '@display' attribute, or configure Mobility for this host.", hostModule->getFullPath().c_str());
            }
            else {
                // ...else, get the initial position from the display string
                radioPos.x = parseInt(hostModule->getDisplayString().getTagArg("p", 0), -1);
                radioPos.y = parseInt(hostModule->getDisplayString().getTagArg("p", 1), -1);
                const char *s = hostModule->getDisplayString().getTagArg("p", 2);
                if (s && *s)
                                error("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                                        " (3rd argument of 'p' tag)"
                                        " from '@display' attribute, or configure Mobility for this host.", hostModule->getFullPath().c_str());
            }
            if (radioPos.x == -1 || radioPos.y == -1)
                /*
                 * tianhao changed due to the position problem of nrphy.
                 * The mobile mecHost cannot get correct position to test the SINR to find the best eNodeg.
                 */        error("The coordinates of '%s' host are invalid. Please set coordinates in "
                        "'@display' attribute, or configure Mobility for this host.", hostModule->getFullPath().c_str());

        }
        myRadioRef = cc->registerRadio(this);
        cc->setRadioPosition(myRadioRef, radioPos);
    }
}

IChannelControl* ChannelAccess::getChannelControl() {
    IChannelControl *cc = dynamic_cast<IChannelControl*>(getSimulation()->findModuleByPath("channelControl"));
    if (!cc)
        throw cRuntimeError("Could not find ChannelControl module with name 'channelControl' in the toplevel network.");
    return cc;
}

/**
 * This function has to be called whenever a packet is supposed to be
 * sent to the channel.
 *
 * This function really sends the message away, so if you still want
 * to work with it you should send a duplicate!
 */
void ChannelAccess::sendToChannel(AirFrame *msg) {
    EV << "sendToChannel: sending to gates\n";

    // delegate it to ChannelControl
    cc->sendToChannel(myRadioRef, msg);
}

//void ChannelAccess::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject*) {
//    EV<<"ChannelAccess::receiveSignal"<<endl;    // since background UEs and their mobility modules are submodules of the e/gNB, a mobilityStateChangedSignal
//    // intended for a background UE would be intercepted by the e/gNB too, making it change its position.
//    // To prevent this issue, we need to check if the source of the signal is the same as the module receiving it
//    if ((strcmp(hostModule->getFullName(), source->getParentModule()->getFullName()) != 0))
//        return;
//
//    if (signalID == inet::IMobility::mobilityStateChangedSignal) {
//        inet::IMobility *mobility = check_and_cast<inet::IMobility*>(obj);
//        radioPos = mobility->getCurrentPosition();
//        positionUpdateArrived = true;
//
//        if (myRadioRef)
//            cc->setRadioPosition(myRadioRef, radioPos);
//    }
//}

/*
 * tianhao changed due to the position problem of nrphy.
 * The mobile mecHost cannot get correct position to test the SINR to find the best eNodeg.
 */
void ChannelAccess::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject*) {
    EV << "ChannelAccess::receiveSignal" << endl;

    // if mobilityStateChangedSignal
    if (signalID == inet::IMobility::mobilityStateChangedSignal) {

        //if it is from mobile mec
        if (strcmp(hostModule->getFullName(), "virtualisationInfrastructure") == 0) {

            cModule *mechost = getParentModule()->getParentModule()->getParentModule();
            if (source->getParentModule() != mechost) {
                EV << "Signal is not from MECHost mobility module, ignoring..." << endl;
                return;
            }
        } else {

            if (strcmp(source->getParentModule()->getFullName(), hostModule->getFullName()) != 0) {
                EV << "Signal is not from the expected hostModule mobility module, ignoring..." << endl;
                return;
            }
        }

        inet::IMobility *mobility = dynamic_cast<inet::IMobility*>(obj);
        if (mobility == nullptr) {
            EV_ERROR << "Received signal object is not of type IMobility, ignoring..." << endl;
            return;
        }

        // update position
        radioPos = mobility->getCurrentPosition();
        positionUpdateArrived = true;

        if (myRadioRef) {
            cc->setRadioPosition(myRadioRef, radioPos);  // update the ChannelControl
        }
    }
}

