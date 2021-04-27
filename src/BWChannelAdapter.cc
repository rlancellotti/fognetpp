#include "BWChannelAdapter.h"

namespace fog {

Define_Module(BWChannelAdapter);

BWChannelAdapter::BWChannelAdapter()
{
    channelFreeMsg = nullptr;
}

BWChannelAdapter::~BWChannelAdapter()
{
    cancelAndDelete(channelFreeMsg);
}

void BWChannelAdapter::initialize()
{
    droppedSignal = registerSignal("dropped");
    queueLengthSignal = registerSignal("queueLength");
    emit(queueLengthSignal, 0);
    busySignal = registerSignal("busy");
    emit(busySignal, false);
    channelFreeMsg = new cMessage("channel-free");
    capacity = par("capacity");
    queue.setName("queue");
    outChannel = getOutChannel();
    if (outChannel == nullptr){
        EV<< "WARNING: Channel Adapter is not needed!" << endl;
    }
}


cChannel * BWChannelAdapter::getOutChannel(){
    return gate(findGate("out"))->findTransmissionChannel();
}

void BWChannelAdapter::handleMessage(cMessage *msg)
{
    if (outChannel == nullptr){
        // just an ideal or delay channel
        send(msg, "out");
        return;
    }
    if (msg == channelFreeMsg) {
        if (queue.isEmpty()) {
            emit(busySignal, false);
        }
        else {
            simtime_t waitingTime, arrivalTime;
            // FIXME: get packet and send it on the channel
            cMessage *pkt = check_and_cast<cMessage *>(queue.pop());
            send(pkt, "out");
            emit(queueLengthSignal, queue.getLength());
            waitingTime=simTime() - arrivalTimestamp[pkt->getId()];
            arrivalTimestamp.erase(pkt->getId());
            emit(queueWaitSignal, waitingTime);
            scheduleAt(outChannel->getTransmissionFinishTime(), channelFreeMsg);
        }
    }
    else {
        bool busy = outChannel->isBusy();
        if (! busy) {
            // channel was idle
            emit(busySignal, true);
            send(msg, "out");
            scheduleAt(outChannel->getTransmissionFinishTime(), channelFreeMsg);
        }
        else {
            // check for container capacity
            if (capacity >= 0 && queue.getLength() >= capacity) {
                emit(droppedSignal, 1);
                delete msg;
                return;
            }
            queue.insert(msg);
            emit(queueLengthSignal, queue.getLength());
            arrivalTimestamp[msg->getId()]=simTime();
        }
    }
}

void BWChannelAdapter::finish()
{
}

}; //namespace

