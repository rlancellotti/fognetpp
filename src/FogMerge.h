#include <omnetpp.h>

#ifndef __FOG_MERGE_H
#define __FOG_MERGE_H
using namespace omnetpp;

namespace fog {

/**
 * All messages received on any input gate will be sent out on the output gate
 */
class FogMerge : public cSimpleModule
{
    protected:
        virtual void handleMessage(cMessage *msg) override;
};

}; //namespace

#endif

