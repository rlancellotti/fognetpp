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
    extChannel = getExtChannel();
    externalGateId=findGate("external$i");
    internalGateId=findGate("internal$i");
    if (extChannel == nullptr){
        EV<< "WARNING: Channel Adapter is not needed!" << endl;
    }
}


cChannel * BWChannelAdapter::getExtChannel(){
    return gate(findGate("external$o"))->findTransmissionChannel();
}

void BWChannelAdapter::handleMessage(cMessage *msg)
{
    // internal message: channel is now free
    if (msg == channelFreeMsg) {
        if (queue.isEmpty()) {
            emit(busySignal, false);
        }
        else {
            simtime_t waitingTime, arrivalTime;
            // FIXME: get packet and send it on the channel
            cMessage *pkt = check_and_cast<cMessage *>(queue.pop());
            send(pkt, "external$o");
            emit(queueLengthSignal, queue.getLength());
            waitingTime=simTime() - arrivalTimestamp[pkt->getId()];
            arrivalTimestamp.erase(pkt->getId());
            emit(queueWaitSignal, waitingTime);
            scheduleAt(extChannel->getTransmissionFinishTime(), channelFreeMsg);
        }
    }
    // traffic external->internal passes without changes
    if (msg->getArrivalGateId() == externalGateId){
        send(msg, "internal$o");
        return;
    }
    // traffic internal->external is queued if channel is a BW channel
    if (msg->getArrivalGateId() == internalGateId){
        if (extChannel == nullptr){
            // just an ideal or delay channel
            send(msg, "external$o");
            return;
        } else {
            bool busy = extChannel->isBusy();
            if (! busy) {
                // channel was idle
                emit(busySignal, true);
                send(msg, "external$o");
                scheduleAt(extChannel->getTransmissionFinishTime(), channelFreeMsg);
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
}

void BWChannelAdapter::finish()
{
}

}; //namespace

