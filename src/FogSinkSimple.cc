#include "FogJob_m.h"
#include "FogSinkSimple.h"

namespace fog {

Define_Module(FogSinkSimple);

void FogSinkSimple::initialize()
{
    responseTimeSignal = registerSignal("responseTime");
    totalQueueingTimeSignal = registerSignal("totalQueueingTime");
    queuesVisitedSignal = registerSignal("queuesVisited");
    totalServiceTimeSignal = registerSignal("totalServiceTime");
    totalDelayTimeSignal = registerSignal("totalDelayTime");
    delaysVisitedSignal = registerSignal("delaysVisited");
    totalBalancerTimeSignal = registerSignal("totalBalancerTime");
	balancersVisitedSignal = registerSignal("balancersVisited");

    //generationSignal = registerSignal("generation");
    //keepJobs = par("keepJobs");
}

void FogSinkSimple::handleMessage(cMessage *msg)
{
    FogJob *job = check_and_cast<FogJob *>(msg);

    // gather statistics
    emit(responseTimeSignal, simTime()- job->getCreationTime());
    emit(totalQueueingTimeSignal, job->getQueuingTime());
    emit(queuesVisitedSignal, job->getQueueCount());
    emit(totalServiceTimeSignal, job->getServiceTime());
    emit(totalDelayTimeSignal, job->getDelayTime());
    emit(delaysVisitedSignal, job->getDelayCount());
    emit(totalBalancerTimeSignal, job->getBalancerTime());
    emit(balancersVisitedSignal, job->getBalancerCount());

    //emit(generationSignal, job->getGeneration());

    //if (!keepJobs)
    delete msg;
}

void FogSinkSimple::finish()
{
    // TODO missing scalar statistics
}

}; //namespace

