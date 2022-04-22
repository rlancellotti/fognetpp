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

package fog.simulations;
import ned.DelayChannel;
import ned.DatarateChannel;

import fog.FogSinkSimple;
import fog.FogSource;
import fog.BWChannelAdapter;


//
// Sample network, consisting of a source and a sink.
//
network TestChannelAdapter
{
    
    
    parameters:
        int BW @unit(bps) = default(1Mbps); 
    submodules:
       
        source[2]: FogSource;
        sink[1]: FogSinkSimple;
        adapter[2]: BWChannelAdapter;
    connections allowunconnected:
        //for i=0..4 {
        //    source[i].out --> adapter[i].in;
        //}
        source[0].out --> adapter[0].internal$i;
        source[1].out --> adapter[1].external$i;
		//adapter[0].out --> sink[0].in++;
        //adapter[1].out --> {delay = 1s;} -->sink[1].in++;
        //adapter[2].out --> {delay = 5s;} -->sink[2].in++;
       	//adapter[3].out --> {delay = 0s; datarate = 2E6 bps;} -->sink[3].in++;
       	adapter[0].external$o --> {delay = 0s; datarate = parent.BW;} -->sink[0].in++;
       	adapter[1].internal$o --> sink[0].in++;
}
