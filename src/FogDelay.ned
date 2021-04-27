
package fog;

simple FogDelay
{
    parameters:
        //@group(Queueing);
        //@signal[delayedJobs](type="int");
        //@statistic[delayedJobs](title="number of delayed jobs";record=vector?,timeavg,max;interpolationmode=sample-hold);
        @display("i=block/delay");
        volatile double delay @unit(s) = default(-1s); // the requested delay time (can be a distribution)
        double maxDelay @unit(s) = default(-1s); // upper bound
        double timeout @unit(s) = default(-1s);
        
    gates:
        input congestionControl;   		// congestion control messages 
        input in[];                     // the incoming message gates
        output out;                     // outgoing message gate
}
