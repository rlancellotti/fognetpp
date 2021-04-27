#include "FogDelay.h"

#include "FogJob_m.h"
#include "CongestionUpdate_m.h"
#include "FogSource.h"

#define CONGESTIONGATENAME "congestionControl"


namespace fog {

Define_Module(FogDelay);

void FogDelay::initialize() {
    currentlyStored = 0;
    maxDelay=par("maxDelay");
    timeout=par("timeout");
    //delayedJobsSignal = registerSignal("delayedJobs");
    //emit(delayedJobsSignal, 0);
    //WATCH(currentlyStored);
    congested=false;
    congestionMultiplier=1.0;
    jobs.clear();
    totalJobs=0;
    droppedJobsTimeout=0;
}

void FogDelay::processNewCloudAppJob(cMessage *msg){
    EV << "CloudDelayCenter::processNewCloudAppJob()" << endl;
    FogJob *job = check_and_cast<FogJob *>(msg);
    // if it is not a self-message, send it to ourselves with a delay
    currentlyStored++;
    totalJobs++;
    simtime_t delay;
    simtime_t now=simTime();
    if (job->getSuggestedDelay()>0)
    {
        delay = job->getSuggestedDelay();
    } else {
        delay = par("delay").doubleValue();
    }
    if (maxDelay>0 && delay>maxDelay) {delay=maxDelay;}
    delay=delay * congestionMultiplier;
    jobs[job->getId()]=job;
    scheduleAt(now + delay, job);
    /*if (timeout>0){
        EV << "Setting up timeout" <<endl;
        // create new timeout message (name=TIMEOUTMESSAGENAME)
        cMessage *to=new cMessage(TIMEOUTMESSAGENAME);
        // FIXME: cannot add a message that is already scheduled
        // add job to timeout message
        to->addObject(job);
        // add timeout message to job
        job->addObject(to);
        // schedule at now+timeout
        scheduleAt(now+timeout, to);
    }*/
}

void FogDelay::processReturningCloudAppJob(cMessage *msg){
    EV << "CloudDelayCenter::processReturningCloudAppJob()" << endl;
    FogJob *job = check_and_cast<FogJob *>(msg);
    job->setDelayCount(job->getDelayCount()+1);
    simtime_t d = simTime() - job->getSendingTime();
    job->setDelayTime(job->getDelayTime() + d);
    // if it was a self message (ie. we have already delayed) so we send it out
    currentlyStored--;
    jobs.erase(job->getId());
    /*// remove timeout from job
    cObject * to=job->removeObject(TIMEOUTMESSAGENAME);
    if (to != NULL){
        // cancel and delete timeout event
        cancelAndDelete(check_and_cast<cMessage *>(to));
    }*/
    // if timeout --> discard job
    if (!checkTimeoutExpired(check_and_cast<FogJob *>(job))){
        send(job, "out");
    }
}


bool FogDelay::checkTimeoutExpired(FogJob *job, bool autoremove){
    if (job==NULL){return false;}
    simtime_t now=simTime();
    if ((timeout>0) && (now - job->getStartTime()>timeout)){
        if (autoremove){
            //EV << "Dropping job from checkTimeoutExpired()";
            // drop and increase droppedJobTimeout
            droppedJobsTimeout++;
            delete job;
        }
        return true;
    }
    return false;
}

void FogDelay::processCongestionUpdateMessage(cMessage *msg){
    EV << "FogDelay::processCongestionUpdateMessage()" << endl;
    CongestionUpdate *umsg=check_and_cast<CongestionUpdate *>(msg);
    double oldCongestionMultiplier=congestionMultiplier;
    jobMapIterator it;
    congested=umsg->getCongestion();
    if (congested){
        congestionMultiplier=umsg->getMultiplier();
    } else {
        congestionMultiplier=1.0;
    }
    // dispose of congestionUpdate message
    delete umsg;
    if (currentlyStored >0){
        // iterate over waiting jobs and update remaining time
        for(it = jobs.begin(); it != jobs.end(); it++) {
            // it->first = key
            // it->second = value
            FogJob *job=check_and_cast<FogJob *>(it->second);
            cancelEvent(job);
            simtime_t remainingTime=job->getArrivalTime() - simTime();
            scheduleAt(simTime()+(remainingTime * congestionMultiplier/oldCongestionMultiplier), job);
        }
    }
}

void FogDelay::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        processReturningCloudAppJob(msg);
    } else {
        if (strcmp(msg->getArrivalGate()->getName(), CONGESTIONGATENAME)==0) {
            processCongestionUpdateMessage(msg);
        } else {
            processNewCloudAppJob(msg);
        }
    }
}
void FogDelay::finish()
{
    // totalJobs, droppedJobs
    recordScalar("totalJobs", totalJobs);
    recordScalar("droppedJobsTimout", droppedJobsTimeout);
    recordScalar("droppedJobsTotal", droppedJobsTimeout);
}

}; //namespace

