
package fog;

//
// Joins several incoming connections. Anything that comes in
// will be sent out immediately on the "out" gate.
//
simple FogMerge
{
    parameters:
        @group(Queueing);
        @display("i=block/join");
    gates:
        input in[];
        output out;
}
