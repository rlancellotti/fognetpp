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
// Generates messages with a configurable interarrival time.
//
simple MultiSource
{
    parameters:
        @display("i=block/source");
        volatile double sendInterval @unit(s) = default(exponential(1s));
        // list of service times for the services.
        volatile double suggestedTime_1 @unit(s) = default(-1s);
        volatile double suggestedTime_2 @unit(s) = default(-1s);
        volatile double suggestedTime_3 @unit(s) = default(-1s);
        volatile double suggestedTime_4 @unit(s) = default(-1s);
        volatile double suggestedTime_5 @unit(s) = default(-1s);
        volatile double suggestedTime_6 @unit(s) = default(-1s);
        volatile double suggestedTime_7 @unit(s) = default(-1s);
        volatile double suggestedTime_8 @unit(s) = default(-1s);
        volatile double suggestedTime_9 @unit(s) = default(-1s);
        volatile double suggestedTime_10 @unit(s) = default(-1s);
        // exit link after each service
        // -1 means go to exit
        // 0 means remain on same node
        int output_1 = default -1; 
        int output_2 = default -1; 
        int output_3 = default -1; 
        int output_4 = default -1; 
        int output_5 = default -1; 
        int output_6 = default -1; 
        int output_7 = default -1; 
        int output_8 = default -1; 
        int output_9 = default -1; 
        int output_10 = default -1; 
        double suggestedDeadline @unit(s) = default(-1s);
        double maxInterval @unit(s) = default(-1s);
        int appId = default(0);
        string jobName =default("job");  
        @signal[sentJob](type="long");
        @statistic[sentJob](title="sent event";record=vector?,count;interpolationmode=none);
                
    gates:
        output out;
}
