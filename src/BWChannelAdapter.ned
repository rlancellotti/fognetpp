package fog;

//
// A queue to manage access to a channel that may be busy
//
simple BWChannelAdapter
{
    parameters:
        @display("i=block/queue;q=queue");
        @signal[dropped](type="long");
        @signal[queueLength](type="long");
        @signal[queueWait](type="simtime_t");
        @signal[busy](type="bool");
        @statistic[dropped](title="drop event";record=vector?,count;interpolationmode=none);
        @statistic[queueLength](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[queueWait](title="queue waiting time";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);

        int capacity = default(-1);    // negative capacity means unlimited queue
    gates:
        inout internal;
        inout external;
}
