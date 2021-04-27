package fog;

//
// A queue with a built-in server
//
simple FogPU
{
    parameters:
        //@group(Queueing);
        @display("i=block/queue;q=queue");
        @signal[dropped](type="long");
        @signal[queueLength](type="long");
        @signal[load](type="double");
        @signal[queueingTime](type="simtime_t");
        @signal[busy](type="bool");
        @statistic[dropped](title="drop event";record=vector?,count;interpolationmode=none);
        @statistic[queueLength](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[load](title="advertised load";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[queueingTime](title="queueing time at dequeue";record=vector?,mean,max;unit=s;interpolationmode=none);
        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);

        int capacity = default(-1);    // negative capacity means unlimited queue
        bool fifo = default(true);     // whether the module works as a queue (fifo=true) or a stack (fifo=false)
        volatile double serviceTime @unit(s) = default(exponential(0.9s));
        double maxServiceTime @unit(s) = default(-1s);
        double timeSlice @unit(s) = default(0.1s);
        double timeout @unit(s) = default(-1s);
        double speedup = default(1.0);	//speedup: suggeste time is divided by speedup value
    gates:
        input congestionControl;
        input in[];
        output out;
        output loadUpdate;
}
