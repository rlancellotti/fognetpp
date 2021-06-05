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
simple FogSource
{
    parameters:
        @display("i=block/source");
        volatile double sendInterval @unit(s) = default(exponential(1s));
        // parameters for face detection task 
        volatile double suggestedTime @unit(s) = default(-1s);
        volatile double suggestedDelay @unit(s) = default(-1s);
        volatile int packetLength @unit(byte) = default(4KiB);
        double SLAmult = default(10);
        double maxInterval @unit(s) = default(-1s);
        int appId = default(0);
        bool realTime = default(false); //If 1 -> realtime 
        string jobName =default("job");  
        @signal[sentJob](type="long");
        @statistic[sentJob](title="sent event";record=vector?,count;interpolationmode=none);
        
        
    gates:
        output out;
}
