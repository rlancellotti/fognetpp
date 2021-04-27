package fog;

moduleinterface IFogNode{
    gates:
        output out;
        input in[];
        output toNeighbors[];
        input fromNeighbors[];
}
//
// A subset of VMs processing requests of the same app
//

module FogNode like IFogNode
{
    parameters:
        @display("i=block/network");
        int nServers;
        int queue = default(10);        
        server[*].maxServiceTime = default(-1s);
        server[*].capacity=queue;
        dispatcher.queue=queue;
       	//server.fifo=true
       	//server.capacity=-1
    gates:
        output out;
        input in[];
        output toNeighbors[];
        input fromNeighbors[];
    submodules:
        dispatcher: <default(FogLoadBalancer)> like IFogLoadBalancer;
        server[nServers]: FogPU;
        merge: FogMerge;
        adapter[sizeof(fromNeighbors)]: BWChannelAdapter;
    connections allowunconnected:
        in[i] --> dispatcher.in++ for i=0..sizeof(in)-1;
        for i=0..sizeof(toNeighbors)-1 {
        	dispatcher.loadToNeighbor++ --> adapter[i].in; 
        	adapter[i].out --> toNeighbors[i];
        } 
        fromNeighbors[i] --> dispatcher.loadFromNeighbor++ for i=0..sizeof(fromNeighbors)-1;
        for i=0..nServers-1 {
            dispatcher.out++ --> server[i].in++;
            server[i].loadUpdate --> dispatcher.localLoad++;
            server[i].out --> merge.in++;
        }
        merge.out --> out;
}
