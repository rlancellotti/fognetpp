#ifndef __FOGPU_H
#define __FOGPU_H

#include <omnetpp.h>
#include "../../queueinglib/QueueingDefs.h"
#include <omnetpp/cmsgpar.h>
#include <omnetpp/cqueue.h>
#include "FogJob_m.h"
#include "LoadUpdate_m.h"

using namespace omnetpp;

namespace fog {

//class CloudAppJob;

/**
 * Abstract base class for single-server queues.
 */
class FogPU : public cSimpleModule
{
    private:
        simsignal_t droppedSignal;
        simsignal_t queueLengthSignal;
        simsignal_t queueingTimeSignal;
        simsignal_t busySignal;
        simsignal_t loadSignal;


        FogJob *jobServiced;
        cMessage *endServiceMsg;
        cMessage *contextSwitchMsg;
        cMessage *timeoutMsg;
        cQueue queue;
        //int capacity;
        bool fifo;
        simtime_t maxServiceTime;
        simtime_t timeout;
        FogJob *getFromQueue();
        // status and stats
        bool busy;
        bool congested;
        double speedup;
        double congestionMultiplier;
        simtime_t totalBusyTime;
        simtime_t startBusyTime;
        simtime_t totalIdleTime;
        simtime_t startIdleTime;
        simtime_t startCongestionTime;
        simtime_t startNCongestionTime;
        simtime_t totalCongestionTime;
        simtime_t totalNCongestionTime;
        double timeSlice;
        int totalJobs;
        int droppedJobsQueue;
        int droppedJobsTimeout;
        // FIXME: during the processing, a job could be dropped.
        // we should handle correctly this event; it's like the timeout case
        // but we record it differently
        int droppedJobsSLA;
        void changeState(int transition);
        void processCongestionUpdateMessage(cMessage *msg);
        void processEndServiceMessage(cMessage *msg);
        void processFogAppJobMessage(cMessage *msg);
        void processContextSwitchMessage(cMessage *msg);
        void processTimeoutMessage(cMessage *msg);
        void setRemainingTime(cMessage *job, simtime_t time);
        simtime_t getRemainingTime(cMessage *job);
        void deleteRemainingTime(cMessage *job);
        void scheduleNextEvent();
        bool checkTimeoutExpired(FogJob *job, bool autoremove=true);
        bool checkSlaExpired(FogJob *job, bool allowremove=true);
        void setTimeout(FogJob *job);
        void cancelTimeout(FogJob *job);
        void removeExpiredJobs();
        void notifyLoad();

    public:
        FogPU();
        virtual ~FogPU();
        static const char *getLoadUpdateName();
        int capacity;
        int length();

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void finish();
        virtual int getCapacity();
        // hook functions to (re)define behaviour
        // virtual void arrival(FogJob *job);
        virtual simtime_t setupService(FogJob *job);
        virtual void endService(FogJob *job);
        virtual void resumeService(FogJob *job);
        virtual void stopService(FogJob *job);
};

}; //namespace

#endif
