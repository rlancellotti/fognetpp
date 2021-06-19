//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package fog;

//
// TODO auto-generated module
//

moduleinterface IFogLoadBalancer{
    parameters:
        int max_hop;
        bool bounceBack;
        int queue;
        string loadOracleName;
        volatile int probeQueryPacketLength @unit(byte);
        volatile int probeAnswerPacketLength @unit(byte);
    gates:
        input in[];
        input localLoad[];
        //input loadFromNeighbor[];
        inout neighbor[];
        output out[];
}

simple FogLoadBalancer like IFogLoadBalancer
{
    parameters:
        int max_hop=default(-1);
        bool bounceBack=default(false);
        int queue = default(10);  
        volatile int probeQueryPacketLength @unit(byte) = default(4KiB);
        volatile int probeAnswerPacketLength @unit(byte) = default(4KiB);
        string loadOracleName = default("None");
        @display("i=block/fork");
        @signal[dropped](type="long");
        @signal[load](type="double");
        @signal[job](type="long");
        @signal[jobFromSrc](type="long");
        @signal[jobFromNeighbor](type="long");
        @signal[jobForward](type="long");
        @signal[jobLocal](type="long");
        @signal[probeQuery](type="long");
        @signal[probeAnswer](type="long");
        @signal[bouncedBack](type="long");
        @signal[jobQueryLoadError](type="double");
        @signal[jobAbsQueryLoadError](type="double");
        @signal[jobForwardPort](type="long");
        
        @statistic[dropped](title="drop event";record=vector?,count;interpolationmode=none);
        @statistic[load](title="load";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[job](title="jobs";record=vector?,stats;interpolationmode=none);
        @statistic[jobFromSrc](title="jobs";record=vector?,stats;interpolationmode=none);
        @statistic[jobFromNeighbor](title="jobs";record=vector?,stats;interpolationmode=none);
        @statistic[jobForward](title="forwarded-jobs";record=vector?,stats;interpolationmode=none);
        @statistic[jobLocal](title="forwarded-jobs";record=vector?,stats;interpolationmode=none);
        @statistic[probeQuery](title="probe-queries";record=vector?,stats;interpolationmode=none);
        @statistic[probeAnswer](title="probe-answers";record=vector?,stats;interpolationmode=none);
        @statistic[bouncedBack](title="bounced-back-jobs";record=vector?,stats;interpolationmode=none);
        @statistic[jobQueryLoadError](title="QueryLoadError";record=vector?,stats;interpolationmode=none);
        @statistic[jobAbsQueryLoadError](title="AbsQueryLoadError";record=vector?,stats;interpolationmode=none);
        @statistic[jobForwardPort](title="AbsQueryLoadError";record=vector?,stats;interpolationmode=none);
        
    gates:
        input in[];
        input localLoad[];
        //input loadFromNeighbor[];
        //output loadToNeighbor[];
        inout neighbor[];
        output out[];
}
