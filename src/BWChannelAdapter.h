#ifndef __BWCHANNELADAPTER_H
#define __BWCHANNELADAPTER_H

#include <omnetpp.h>
#include "../../queueinglib/QueueingDefs.h"
#include <omnetpp/cmsgpar.h>
#include <omnetpp/cqueue.h>
#include "FogJob_m.h"
#include "LoadUpdate_m.h"

namespace fog {

class BWChannelAdapter : public cSimpleModule
{
    private:
		simsignal_t queueLengthSignal;
        simsignal_t queueWaitSignal;
		simsignal_t busySignal;
        simsignal_t droppedSignal;

        cPacket *pkt;
        cMessage *channelFreeMsg;
        cQueue queue;
        cChannel *outChannel;
        int capacity;
        std::map<int, simtime_t> arrivalTimestamp;


    public:
        BWChannelAdapter();
        virtual ~BWChannelAdapter();

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;

    private:
        cChannel *getOutChannel();

};

}; //namespace

#endif
