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
import fog.VariableDelayChannel;
import fog.IFogNode;
import fog.FogNode;
import fog.FogAppClassifier;
import fog.FogSinkSimple;
import fog.FogSource;
import fog.LoadOracle;


//
// Sample network, consisting of a source and a sink.
//


network FogMeshNetworkDelay
{
	parameters:
        int nSources = default(5);
        int nClasses = default(1);
		// FIXME: must use variable delay 
        double linkdelay @unit(s) = default(1s);
        source[*].suggestedTime=default(truncnormal(1s, 0.1s));
        source[*].sendInterval=default(exponential(1.1s));
        fog[*].nServers=default(1);
        fog[*].server[*].serviceTime = -1s;
        fog[*].server[*].timeout = default(-1s); // disable timeout
    types:
        channel C extends VariableDelayChannel {
			delay=truncnormal(linkdelay, 0.1*linkdelay);
        }
    submodules:
        source[nSources]: FogSource;
        fog[nSources]: <default("FogNode")> like IFogNode;
        classifier: FogAppClassifier;
        sink[nClasses+1]: FogSinkSimple;
        loadOracle: LoadOracle;
    connections:
        for i=0..nSources-1 {
            source[i].out --> fog[i].in++;
            fog[i].out --> classifier.in++;
        }
        for i=0..nClasses-1 {
            classifier.out++ --> sink[i].in++;
        }
        classifier.rest --> sink[nClasses].in++;
        // add delay
        for i=0..nSources-2, for j=i+1..nSources-1 {
            // FIXME: must use variable delay 
            fog[i].toNeighbors++ --> C --> fog[j].fromNeighbors++;
            fog[j].toNeighbors++ --> C --> fog[i].fromNeighbors++;
        }

}
