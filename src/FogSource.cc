//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "FogSource.h"
#include "FogJob_m.h"

namespace fog {

Define_Module(FogSource);

//int FogSource::nextJobId = 1;

FogSource::FogSource()
{
    timerMessage = NULL;
}

FogSource::~FogSource()
{
    cancelAndDelete(timerMessage);
}

void FogSource::initialize()
{
    timerMessage = new cMessage("timer");
    maxInterval = par("maxInterval").doubleValue();
    sentJobSignal = registerSignal("sentJob");
    scheduleAt(simTime(), timerMessage);
}

/**
 * When the timer is triggered, the job is created, initialized and send 'out'
 * Next timer is scheduled
 */
void FogSource::handleMessage(cMessage *msg)
{
    ASSERT(msg==timerMessage);
    simtime_t t, trand;
    // create new message
    FogJob *job = new FogJob(getJobName());
    job->setStartTime(simTime());
    job->setQueuingTime(0.0);
    job->setServiceTime(0.0);
    job->setDelayTime(0.0);
    job->setBalancerTime(0.0);
    job->setSuggestedTime(par("suggestedTime").doubleValue());
    job->setSuggestedDelay(par("suggestedDelay").doubleValue());
    job->setQueueCount(0);
    job->setDelayCount(0);
    job->setBalancerCount(0);
    job->setSlaDeadline(simTime()+job->getSuggestedTime()*par("SLAmult"));
    job->setAppId(par("appId"));
    job->setRealTime(par("realTime"));
    job->setByteLength(par("packetLength"));
    job->setPreviousLoad(-1);
    job->setExpectedLoad(-1);
    //EV << "ID is: "<< job->getId() <<endl;
    //job->setId(nextJobId++);
    send(job, "out");
    // schedule next message
    trand=par("sendInterval").doubleValue();
    if (maxInterval>0 && trand>maxInterval){
        t=simTime() + maxInterval;
    } else {
        t=simTime() + trand;
    }
    emit(sentJobSignal, 1);
    scheduleAt(t, timerMessage);
}

const char *FogSource::getJobName(){
    return par("jobName");
}


}; // namespace
