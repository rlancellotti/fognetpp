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
// Consumes received messages and collects statistics
//
simple FogSinkSimple
{
    parameters:
        @display("i=block/sink");
        //@group(Queueing);
        @signal[responseTime](type="simtime_t");
        @signal[totalQueueingTime](type="simtime_t");
        @signal[totalDelayTime](type="simtime_t");
        @signal[totalServiceTime](type="simtime_t");
        @signal[totalBalancerTime](type="simtime_t");
        @signal[queuesVisited](type="long");
        @signal[delaysVisited](type="long");
        @signal[balancersVisited](type="long");

        @statistic[responseTime](title="lifetime of arrived jobs";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[totalQueueingTime](title="the total time spent in queues by arrived jobs";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[totalDelayTime](title="the total time spent in delay nodes by arrived jobs";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[totalServiceTime](title="the total time spent being processed by arrived jobs";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[totalBalancerTime](title="the total time spent in a balancer by arrived jobs";unit=s;record=vector?,stats;interpolationmode=none);
        @statistic[queuesVisited](title="the total number of queues visited by arrived jobs";record=vector?,stats;interpolationmode=none);
        @statistic[delaysVisited](title="the total number of delays visited by arrived jobs";record=vector?,stats;interpolationmode=none);
        @statistic[balancersVisited](title="the total number of delays visited by arrived jobs";record=vector?,stats;interpolationmode=none);

    gates:
        input in[];
}
