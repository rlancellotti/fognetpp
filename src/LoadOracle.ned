
package fog;

//
// A queue with a built-in server
//
simple LoadOracle
{
    parameters:
        @display("i=block/table2");
        //@signal[dropped](type="int");
        //@signal[queueLength](type="int");
        //@signal[queueingTime](type="simtime_t");
        //@signal[busy](type="bool");
        //@statistic[dropped](title="drop event";record=vector?,count;interpolationmode=none);
        //@statistic[queueLength](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
        //@statistic[queueingTime](title="queueing time at dequeue";record=vector?,mean,max;unit=s;interpolationmode=none);
        //@statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);

    gates:
        input loadUpdate @loose;
}
