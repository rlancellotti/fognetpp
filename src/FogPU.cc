//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <exception>
#include "FogPU.h"
#include "FogJob_m.h"
#include "CloudCongestionUpdate_m.h"
#include "msgTypes.h"

using namespace omnetpp;

// transitions of server state
#define BUSY2IDLE 1
#define IDLE2BUSY 2
#define BUSY2BUSY 3
#define IDLE2IDLE 4
#define ANY2IDLE 5
#define ANY2BUSY 6

// should be the same name of .ned file
#define CONGESTIONGATENAME "congestionControl"

#define REMAININGTIME "remaining-time"

namespace fog {

Define_Module(FogPU);

FogPU::FogPU()
{
    jobServiced = NULL;
    endServiceMsg = NULL;
    contextSwitchMsg = NULL;
    timeoutMsg = NULL;
}

FogPU::~FogPU()
{
    delete jobServiced;
    cancelAndDelete(endServiceMsg);
    cancelAndDelete(contextSwitchMsg);
    cancelAndDelete(timeoutMsg);
    // ???? 
    int OwnedSize = this->defaultListSize();
    for (int i = 0; i < OwnedSize; i++)
    {
        cOwnedObject *Del = (cOwnedObject *) this->defaultListGet(0);
        this->drop(Del);
        if ((strcmp(Del->getName(), "loadUpdate") == 0))
        {
            delete Del;
            Del = NULL;
        }
    }

}

void FogPU::initialize()
{
    droppedSignal = registerSignal("dropped");
    queueingTimeSignal = registerSignal("queueingTime");
    loadSignal = registerSignal("load");
    emit(loadSignal, 0.0);
    queueLengthSignal = registerSignal("queueLength");
    emit(queueLengthSignal, 0);
    busySignal = registerSignal("busy");
    emit(busySignal, false);
    busy = false;
    congested = false;
    congestionMultiplier = 1.0;
    totalBusyTime = 0;
    startBusyTime = 0;
    totalIdleTime = 0;
    startIdleTime = simTime();
    startCongestionTime = 0;
    startNCongestionTime = simTime();
    totalCongestionTime = 0;
    totalNCongestionTime = 0;
    totalJobs = 0;
    droppedJobsQueue = 0;
    droppedJobsTimeout = 0;
    droppedJobsSLA = 0;
    endServiceMsg = new cMessage("end-service");
    contextSwitchMsg = new cMessage("context-switch");
    timeoutMsg = new cMessage("timeout");
    fifo = par("fifo");
    capacity = par("capacity");
    timeSlice = par("timeSlice");
    queue.setName("queue");
    maxServiceTime = par("maxServiceTime");
    timeout = par("timeout");
    speedup = par("speedup");
}

/**
 * Schedule next event
 * If remainingTime < timeSlice (that is, we are almost finished) we schedule the end of the job event
 * otherwise, we schedule the context switch.
 */
void FogPU::scheduleNextEvent()
{
    // Schedule context switch or end of service for current job
    simtime_t remainingTime, now;
    now = simTime();
    remainingTime = getRemainingTime(jobServiced);
    if ((remainingTime <= timeSlice) || (timeSlice <= 0))
    {
        EV<<"EndMessage: ";
        EV<<"RemainingTime: "<<remainingTime<<" schedule at: "<<now+(remainingTime*congestionMultiplier)<<endl;
        setRemainingTime(jobServiced, remainingTime);
        scheduleAt(now + (remainingTime * congestionMultiplier), endServiceMsg);
    }
    else
    {
        EV<<"ContextSwitch: ";
        EV<<"RemainingTime: "<<remainingTime<<" schedule at: "<<now+(timeSlice*congestionMultiplier)<<endl;
        setRemainingTime(jobServiced, remainingTime - timeSlice);
        scheduleAt(now + (timeSlice * congestionMultiplier), contextSwitchMsg);
    }
}

void FogPU::processCloudCongestionUpdateMessage(cMessage *msg)
{
    CloudCongestionUpdate *umsg = check_and_cast<CloudCongestionUpdate *>(msg);
    double oldCongestionMultiplier = congestionMultiplier;
    EV << "processCloudCongestionUpdateMessage(cong=" << umsg->getMultiplier() << ")\n";
    simtime_t remainingTime = 0.0;
    simtime_t now;
    now = simTime();
    if (congested)
    {
        totalCongestionTime = totalCongestionTime + (now - startCongestionTime);
    }
    else
    {
        totalNCongestionTime = totalNCongestionTime + (now - startNCongestionTime);
    }
    congested = umsg->getCongestion();
    if (congested)
    {
        congestionMultiplier = umsg->getMultiplier();
        startCongestionTime = now;
    }
    else
    {
        congestionMultiplier = 1.0;
        startNCongestionTime = now;
    }
    // dispose of congestionUpdate message
    delete umsg;
    if (busy)
    {
        if (endServiceMsg->isScheduled())
        {
            remainingTime = endServiceMsg->getArrivalTime() - now;
            cancelEvent(endServiceMsg);
            scheduleAt(now + (remainingTime * congestionMultiplier / oldCongestionMultiplier), endServiceMsg);
        }
        if (contextSwitchMsg->isScheduled())
        {
            remainingTime = contextSwitchMsg->getArrivalTime() - now;
            cancelEvent(contextSwitchMsg);
            scheduleAt(now + (remainingTime * congestionMultiplier / oldCongestionMultiplier), contextSwitchMsg);
        }
    }
}

/**
 * Context switch )a job is executing).
 * If queue is not empty, we exchange the currently executing job with another one in the queue.
 */
void FogPU::processContextSwitchMessage(cMessage *msg)
{
    //EV << "processContextSwitchMessage " << msg << "(job serviced=" << jobServiced->getId() << ", qlen=" << queue.getLength() << ")\n";
    removeExpiredJobs();
    if (!queue.isEmpty())
    {
        FogJob *oldJob;
        // stop service of current job
        stopService(jobServiced);
        // context switch
        oldJob = jobServiced;
        jobServiced = getFromQueue();
        queue.insert(oldJob);
        // resume service of new job
        resumeService(jobServiced);
    }
    scheduleNextEvent();
}

void FogPU::processEndServiceMessage(cMessage *msg)
{
    EV << "processEndServiceMessage " << msg << " (job serviced " << jobServiced->getId() << ")\n";
    removeExpiredJobs();

    endService(jobServiced);

    if (queue.isEmpty())
    {

        jobServiced = NULL;
        //emit(busySignal, 0);
        changeState(ANY2IDLE);
    }
    else
    {
        jobServiced = getFromQueue();
        emit(queueLengthSignal, length());
        resumeService(jobServiced);
        EV << "processEndAppJobMessage: (new service time=" << getRemainingTime(jobServiced) << " new job is " << jobServiced->getId() << ")\n";
        changeState(ANY2BUSY);
        scheduleNextEvent();
    }
}

void FogPU::processFogAppJobMessage(cMessage *msg)
{
    EV << "processFogAppJobMessage " << msg << "\n";
    FogJob *job = check_and_cast<FogJob *>(msg);
    setupService(job);
    // if timeout is expired, simply drop the job and continue
    totalJobs++;
    if (checkTimeoutExpired(job) || checkSlaExpired(job))
    {
        return;
    }
    if (!jobServiced)
    {
        // processor was idle
        jobServiced = job;
        //emit(busySignal, true);
        changeState(ANY2BUSY);
        EV << "processFogAppJobMessage: service=" << getRemainingTime(jobServiced) << " job is " << jobServiced->getAppId() << "\n";
        resumeService(jobServiced);
        scheduleNextEvent();
    }
    else
    {
        // check for container capacity
        if (capacity >= 0 && queue.getLength() >= capacity)
        {
            EV << "Capacity full! Job dropped.\n";
            notifyLoad();
            //if (ev.isGUI()) bubble("Dropped!");
            emit(droppedSignal, 1);
            droppedJobsQueue++;

            delete job;
            return;
        }
        queue.insert(job);
        //emit(queueLengthSignal, length());
        if (timeSlice <= 0)
        {
            job->setQueueCount(job->getQueueCount() + 1);
        }
    }
}

void FogPU::processTimeoutMessage(cMessage *msg)
{
    EV << "Got Timeout Event";
    // drop current job
    if (checkTimeoutExpired(jobServiced))
    {
        // select new job
        removeExpiredJobs();
        if (queue.isEmpty())
        {
            jobServiced = NULL;
            //emit(busySignal, false);
            //dropUpdate = new cMessage("dropUpdate", 2);
            //send(dropUpdate, "loadUpdate");
            changeState(ANY2IDLE);
        }
        else
        {
            jobServiced = getFromQueue();
            emit(queueLengthSignal, length());
            resumeService(jobServiced);
            EV << "processEndAppJobMessage: (new service time=" << getRemainingTime(jobServiced) << " new job is " << jobServiced->getId() << ")\n";
            //dropUpdate = new cMessage("dropUpdate", 2);
            //send(dropUpdate, "loadUpdate");
            changeState(ANY2BUSY);
            scheduleNextEvent();
        }
        notifyLoad();
    }
}

void FogPU::setRemainingTime(cMessage *job, simtime_t time)
{
    cMsgPar *par;
    if (job->hasPar(REMAININGTIME))
    {
        par = &(job->par(REMAININGTIME));
        par->setDoubleValue(SIMTIME_DBL(time));
    }
    else
    {
        par = new cMsgPar(REMAININGTIME);
        par->setDoubleValue(SIMTIME_DBL(time));
        job->addPar(par);
    }
}

simtime_t FogPU::getRemainingTime(cMessage *job)
{
    cMsgPar par;
    if (job->hasPar(REMAININGTIME))
    {
        par = job->par(REMAININGTIME);
        return (double) par;
    }
    else
    {
        return 0;
    }
}

void FogPU::deleteRemainingTime(cMessage *job)
{
    // remove parameter
    cObject * p = jobServiced->removeObject(REMAININGTIME);
    if (p != NULL)
    {
        delete p;
    }

}

void FogPU::handleMessage(cMessage *msg)
{
    if (msg == timeoutMsg)
    {
        processTimeoutMessage(msg);
    }
    else
    {
        if (msg == endServiceMsg)
        {
            processEndServiceMessage(msg);
        }
        else
        {
            if (msg == contextSwitchMsg)
            {
                processContextSwitchMessage(msg);
            }
            else
            {
                if (strcmp(msg->getArrivalGate()->getName(), CONGESTIONGATENAME) == 0)
                {
                    processCloudCongestionUpdateMessage(msg);
                }
                else
                {
                    processFogAppJobMessage(msg);
                }
            }
        }
    }
    //if (ev.isGUI()) getDisplayString().setTagArg("i",1, !jobServiced ? "" : "cyan3");
}

/**
 * Get a job from queue. If FIFO=True, get from head.
 */
FogJob *FogPU::getFromQueue()
{
    // we assume that no expired jobs are present in the list
    FogJob *job;
    do
    {
        if (fifo)
        {
            job = (FogJob *) queue.pop();
        }
        else
        {
            job = (FogJob *) queue.back();
            // FIXME: this may have bad performance as remove uses linear search
            queue.remove(job);
        }
    } while (checkTimeoutExpired(job) || checkSlaExpired(job));
    emit(queueLengthSignal, length());
    return job;
}

int FogPU::length()
{
    return queue.getLength();
}

int FogPU::getCapacity()
{
    return this->capacity;
}

simtime_t FogPU::setupService(FogJob *job)
{
    // gather initial queuing time statistics
    simtime_t t;
    job->setTimestamp();
    if (job->getSuggestedTime()>0)
    {
        t = job->getSuggestedTime();
    } else {
        t = par("serviceTime").doubleValue();
    }
    // manage speedup parameter
    t /= speedup;
    EV << t << "---SERVICE TIME---" << endl;
    if (maxServiceTime > 0 && t > maxServiceTime)
    {
        t = maxServiceTime;
    }
    EV<<"Service time: "<<t<<endl;
    setRemainingTime(job, t);
    return t;
}

/**
 * When job is finished being processed
 *  > if it isn't expired, ok
 *  > if it expired, it is drop
 */
void FogPU::endService(FogJob *job)
{
    EV << "Finishing service of " << job->getName() << endl;
    //simtime_t d = simTime() - job->getTimestamp();
    //simtime_t d=stopService(job);
    stopService(job);
    //job->setServiceTime(job->getServiceTime() + d);
    deleteRemainingTime(job);
    cancelTimeout(job);
    if (!checkTimeoutExpired(job) && !checkSlaExpired(job))
    {
        send(job, "out");
        changeState(ANY2IDLE);
        notifyLoad();
    }
    /*
    else{
        dropUpdate = new cMessage("dropUpdate", 2);
        send(dropUpdate, "loadUpdate");
    }
    */
}

/**
 * Resume a job from queue
 * Set balancer time (how long it has been in the queue)
 */
void FogPU::resumeService(FogJob *job)
{
    simtime_t now, ts, d;
    now = simTime();
    ts = job->getTimestamp();
    d = now - ts;
    job->setQueuingTime(job->getQueuingTime() + d);
    job->setTimestamp();
    setTimeout(job);
}

/**
 * Stop current process (ex. for context switch)
 * Set time service (how long it has been in execution)
 */
void FogPU::stopService(FogJob *job)
{
    simtime_t now, ts, d;
    now = simTime();
    ts = job->getTimestamp();
    d = now - ts;
    job->setServiceTime(job->getServiceTime() + d);
    job->setTimestamp();
    cancelTimeout(job);
}

/**
 * check if job has reached timeout.
 * If autoremove is set, the job with expired timeout is purged
 */
bool FogPU::checkTimeoutExpired(FogJob *job, bool autoremove)
{
    if (job == NULL)
    {
        return false;
    }
    simtime_t now = simTime();
    if ((timeout > 0) && (now - job->getStartTime() > timeout))
    {
        if (autoremove)
        {
            //EV << "Dropping job from checkTimeoutExpired()";
            // drop and increase droppedJobTimeout
            droppedJobsTimeout++;
            emit(droppedSignal, 1);
            queue.remove(job);
            delete job;
        }
        return true;
    }
    return false;
}

bool FogPU::checkSlaExpired(FogJob *job, bool allowremove)
{
    if (job == NULL)
    {
        return false;
    }
    if (job->getRealTime() && (simTime()>job->getSlaDeadline()))
    {
        //EV << "Dropping job from checkSlaExpired()";
        // drop and increase droppedJobSla
        if (allowremove)
        {
            droppedJobsSLA++;
            emit(droppedSignal, 1);
            queue.remove(job);
            delete job;
        }
        return true;
    }
    return false;
}


/**
 * Set timeout for current job
 */
void FogPU::setTimeout(FogJob *job)
{
    // timeout can occur only for the currently processed job
    simtime_t to;
    if (timeout <= 0)
        return;
    // compute timeout for current job
    to = job->getStartTime() + timeout;
    // set timeout
    scheduleAt(to, timeoutMsg);
}

void FogPU::cancelTimeout(FogJob *job)
{
    if (timeoutMsg->isScheduled())
    {
        cancelEvent(timeoutMsg);
    }
}

/*
 * Remove expired jobs (timeout, queue length, SLA
 */
void FogPU::removeExpiredJobs()
{
    // iterate over queue
    //EV << "FogPU::removeExpiredJobs()..."<< endl;
    std::vector<FogJob *> expiredJobs;  // support vector for expired jobs
                                        // at the end of the loop, they will be removed
    for (cQueue::Iterator i(queue); !i.end(); ++i)
    {
        //CloudAppJob *job=check_and_cast<CloudAppJob *>((*i)++);
        //CloudAppJob *job=check_and_cast<CloudAppJob *>(*i);

        FogJob *job = check_and_cast<FogJob *>(*i);

        // if job is expired the job is removed
        if (checkTimeoutExpired(job, false))
        {
            //EV << "removing job" <<endl;
            expiredJobs.push_back(job);
            // increase droppedJobTimeout
            droppedJobsTimeout++;
        } else {
            if (checkSlaExpired(job, false))
            {
                //EV << "removing job" <<endl;
                expiredJobs.push_back(job);
                // increase droppedJobsSLA
                droppedJobsSLA++;
            }
        }

    }

    //Removing expired jobs
    for (FogJob *job: expiredJobs) {
        queue.remove(job);
        delete job;
    }

    notifyLoad();
    //EV << "done."<< endl;
}

/**
 * Change server status (busy, idle)
 */
void FogPU::changeState(int transition)
{
    //FIXME: add call to loadUpdate when status changes!
    simtime_t now;
    switch (transition) {
        case BUSY2IDLE:
            emit(busySignal, false);
            EV << "Busy -> Idle" << endl;
            now = simTime();
            busy = false;
            startIdleTime = now;
            totalBusyTime += now - startBusyTime;
            break;
        case IDLE2BUSY:
            emit(busySignal, true);
            EV << "Idle -> Busy" << endl;
            now = simTime();
            busy = true;
            startBusyTime = now;
            totalIdleTime += now - startIdleTime;
            break;
        case BUSY2BUSY:
            // nothing to do
            break;
        case IDLE2IDLE:
            // nothing to do
            break;
        case ANY2BUSY:
            if (!busy)
            {
                changeState(IDLE2BUSY);
            } // else {changeState(IDLE2IDLE);}
            break;
        case ANY2IDLE:
            if (busy)
            {
                changeState(BUSY2IDLE);
            } // else {changeState(BUSY2BUSY);}
            break;
    }
    //notifyLoad();
}

void FogPU::finish()
{
    float rho, congRatio;
    simtime_t now;
    now = simTime();
    // server utilization
    if (busy)
    {
        totalBusyTime += now - startBusyTime;
    }
    else
    {
        totalIdleTime += now - startIdleTime;
    }
    rho = totalBusyTime / (totalBusyTime + totalIdleTime);
    recordScalar("rho", rho);
    // congestion ratio
    if (congested)
    {
        totalCongestionTime += now - startCongestionTime;
    }
    else
    {
        totalNCongestionTime += now - startNCongestionTime;
    }
    congRatio = totalCongestionTime / (totalCongestionTime + totalNCongestionTime);
    // FIXME: These metrics should be per class
    recordScalar("congestionRatio", congRatio);
    // totalJobs, droppedJobs
    recordScalar("totalJobs", totalJobs);
    recordScalar("droppedJobsQueue", droppedJobsQueue);
    recordScalar("droppedJobsTimout", droppedJobsTimeout);
    recordScalar("droppedJobsSLA", droppedJobsSLA);
    recordScalar("droppedJobsTotal", droppedJobsQueue + droppedJobsTimeout + droppedJobsSLA);
}

/**
 * Compute local load and send message to LoadBalancer (localLoadUpdate)
 */
void FogPU::notifyLoad() {
    //FIXME: add signal to compare with queue length
    cGate *dispatcherGate = gate("loadUpdate");
    double load = busy+length();
    if (!dispatcherGate->isConnected())
        return;
    LoadUpdate *loadUpdate = new LoadUpdate(getLoadUpdateName(), KIND_LOAD);
    loadUpdate->setLoad(load);
    emit(loadSignal, load);
    if (length() >= capacity && capacity > 0)
        loadUpdate->setQueueFull(1);
    else
        loadUpdate->setQueueFull(0);
    send(loadUpdate, "loadUpdate");
}

const char *FogPU::getLoadUpdateName(){
    return "loadUpdate";
}

}
;
//namespace
