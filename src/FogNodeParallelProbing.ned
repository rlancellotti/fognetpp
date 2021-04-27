package fog;


//
// A subset of VMs processing requests of the same app
//
module FogNodeParallelProbing extends FogNode like IFogNode
{
    parameters:
        dispatcher.typename = "FogParallelProbing";
}
