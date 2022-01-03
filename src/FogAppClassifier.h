#ifndef __FOG_APPCLASSIFIER_H
#define __FOG_APPCLASSIFIER_H

#include <omnetpp.h>

using namespace omnetpp;
namespace fog {

/**
 * See the NED declaration for more info.
 */
class  FogAppClassifier : public cSimpleModule
{
    private:
    protected:
        virtual void handleMessage(cMessage *msg) override;
};

}; //namespace

#endif
