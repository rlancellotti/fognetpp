package fog;


//
// A Fog node with load balancing
//
module FogNodeSequentialForward extends FogNode like IFogNode
{
    parameters:
        dispatcher.typename = "FogSequentialForwarding";
}
