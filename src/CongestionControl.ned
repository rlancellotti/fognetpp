package fog;

simple CongestionControl
{
    parameters:
        @display("i=block/cogwheel");
        volatile double toCongestionStatusInterval @unit(s) = default(exponential(10s));
        volatile double fromCongestionStatusInterval @unit(s) = default(exponential(10s));
        double congestionMultiplier = default(1.3);

    gates:
        output congestionControl[];
}
