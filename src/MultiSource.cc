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

#include "MultiSource.h"
#include "MultiJob_m.h"

namespace fog {

Define_Module(MultiSource);

MultiSource::MultiSource()
{
    timerMessage = NULL;
}

MultiSource::~MultiSource()
{
    cancelAndDelete(timerMessage);
}

void MultiSource::initialize()
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
void MultiSource::handleMessage(cMessage *msg)
{
    ASSERT(msg==timerMessage);
    simtime_t t, trand;
    // create new message
    MultiJob *job = new MultiJob(getJobName());
    job->setStartTime(simTime());
    job->setQueuingTime(0.0);
    job->setServiceTime(0.0);
    job->setDelayTime(0.0);
    // FIXME: consider array nature of times
    //job->setSuggestedTime(par("suggestedTime").doubleValue());
    int srv=0;
    while(true){
        std::string parname = "suggestedTime_" + std::to_string(srv);
        simtime_t val;
        job->setSuggestedTime(srv, par(parname.c_str()).doubleValue());
    }
    job->setQueueCount(0);
    job->setDelayCount(0);
    if (par("suggestedDeadline").doubleValue()>0){
        job->setSlaDeadline(simTime()+par("suggestedDeadline").doubleValue());
    } else {
        job->setSlaDeadline(-1.0);
    }
    job->setByteLength(par("packetLength"));
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

const char *MultiSource::getJobName(){
    return par("jobName");
}


}; // namespace
