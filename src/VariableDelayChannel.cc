
#include "VariableDelayChannel.h"
#include "omnetpp/cmessage.h"
#include "omnetpp/cenvir.h"
#include "omnetpp/globals.h"
#include "omnetpp/cgate.h"
#include "omnetpp/cexception.h"
#include "omnetpp/ctimestampedvalue.h"

namespace fog {

Register_Class(VariableDelayChannel);

simsignal_t VariableDelayChannel::messageSentSignal;
simsignal_t VariableDelayChannel::messageDiscardedSignal;

VariableDelayChannel *VariableDelayChannel::create(const char *name)
{
    return dynamic_cast<VariableDelayChannel *>(cChannelType::getDelayChannelType()->create(name));
}

void VariableDelayChannel::initialize()
{
    messageSentSignal = registerSignal("messageSent");
    messageDiscardedSignal = registerSignal("messageDiscarded");
}

void VariableDelayChannel::rereadPars()
{
    setFlag(FL_ISDISABLED, par("disabled"));
    queryDelayScale = par("queryDelayScale");
}

void VariableDelayChannel::handleParameterChange(const char *)
{
    rereadPars();
}

void VariableDelayChannel::setDelay(double d)
{
    par("delay").setDoubleValue(d);
}

void VariableDelayChannel::setDisabled(bool d)
{
    par("disabled").setBoolValue(d);
}

void VariableDelayChannel::processMessage(cMessage *msg, simtime_t t, result_t& result)
{
    // if channel is disabled, signal that message should be deleted
    if (flags & FL_ISDISABLED) {
        result.discard = true;
        cTimestampedValue tmp(t, msg);
        emit(messageDiscardedSignal, &tmp);
        return;
    }

    // propagation delay modeling
    result.delay = par("delay");
    // scale delay for query messages
    if (queryDelayScale >0 &&
        ((strcmp(msg->getName(), FogLoadBalancer::getAnswerName()) == 0) ||
         (strcmp(msg->getName(), FogLoadBalancer::getProbeQueryName()) == 0))){
        result.delay *= queryDelayScale;
    }

    // emit signals
    if (mayHaveListeners(messageSentSignal)) {
        MessageSentSignalValue tmp(t, msg, &result);
        emit(messageSentSignal, &tmp);
    }
}

}  // namespace fog

