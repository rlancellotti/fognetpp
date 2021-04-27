package fog;


//
// A Fog node with load balancing
//
module FogNodeOracleForward extends FogNode like IFogNode
{
    parameters:
        dispatcher.typename = "FogOracleForwarding";
}
