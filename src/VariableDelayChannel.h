#ifndef __VARIABLEDELAYCHANNEL_H
#define __VARIABLEDELAYCHANNEL_H

#include <omnetpp/cchannel.h>
#include "FogLoadBalancer.h"
using namespace omnetpp;

namespace fog {

/**
 * @brief Channel with variable propagation delay.
 *
 */
class VariableDelayChannel : public cChannel //implies noncopyable
{
  protected:
    static simsignal_t messageSentSignal;
    static simsignal_t messageDiscardedSignal;


  private:
    enum {
      FL_ISDISABLED = 1 << 10,
      FL_DELAY_NONZERO = 1 << 11,
    };
    double probeDelayScale;

  private:
    // internal: checks whether parameters have been set up
    void checkState() const  {if (!parametersFinalized()) throw cRuntimeError(this, E_PARAMSNOTREADY);}

  protected:
    // internal: update cached copies of parameters
    void rereadPars();

    /**
     * Called back when a parameter changes. Redefined from cComponent.
     */
    virtual void handleParameterChange(const char *parname) override;

  public:
    /** @name Constructors, destructor */
    //@{
    /**
     * Constructor. This is only for internal purposes, and should not
     * be used when creating channels dynamically; use the create()
     * factory method instead.
     */
    explicit VariableDelayChannel(const char *name=nullptr) : cChannel(name) {}

    /**
     * Destructor.
     */
    virtual ~VariableDelayChannel() {}

    /**
     * Utility function for dynamic channel creation. Equivalent to
     * <tt>cChannelType::getDelayChannelType()->create(name)</tt>.
     */
    static VariableDelayChannel *create(const char *name);
    //@}

    /** @name Redefined cChannel member functions. */
    //@{
    /**
     * The VariableDelayChannel implementation of this method always returns false.
     */
    virtual bool isTransmissionChannel() const override {return false;}

    /**
     * The VariableDelayChannel implementation of this method always returns zero.
     */
    virtual double getNominalDatarate() const override {return 0;}

    /**
     * The VariableDelayChannel implementation of this method always returns zero.
     */
    virtual simtime_t calculateDuration(cMessage *msg) const override {return 0;}

    /**
     * The VariableDelayChannel implementation of this method always returns zero.
     */
    virtual simtime_t getTransmissionFinishTime() const override {return SIMTIME_ZERO;}

    /**
     * The VariableDelayChannel implementation of this method always returns false.
     */
    virtual bool isBusy() const override {return false;}

    /**
     * The VariableDelayChannel implementation of this method does nothing.
     */
    virtual void forceTransmissionFinishTime(simtime_t t) override {}
    //@}

    /** @name Setting and getting channel parameters. */
    //@{
    /**
     * Sets the propagation delay of the channel, in seconds.
     */
    virtual void setDelay(double d);

    /**
     * Disables or enables the channel.
     */
    virtual void setDisabled(bool d);

    /**
     * Returns the propagation delay of the channel, in seconds.
     * This method is equivalent to reading the "delay" parameter, via par("delay").
     */
    virtual simtime_t getDelay() const {checkState(); return (simtime_t) par("delay");}

    /**
     * Returns whether the channel is disabled. This method is equivalent
     * to reading the "disabled" parameter, via par("disabled").
     * A disabled channel discards all messages sent on it.
     */
    virtual bool isDisabled() const  {checkState(); return flags & FL_ISDISABLED;}
    //@}

    /** @name Implementation methods */
    //@{
    /**
     * Initialization.
     */
    virtual void initialize() override;

    /**
     * This implementation delivers the message to the opposite gate
     * with a delay.
     */
#if OMNETPP_VERSION >= 0x0600
    virtual Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
#else
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result) override;
#endif

    //@}
};

}  // namespace fog


#endif


