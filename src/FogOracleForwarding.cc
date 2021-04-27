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

#include "FogLoadBalancer.h"
#include "FogOracleForwarding.h"
#include "FogJob_m.h"

/**
 * Sequential Forward:
 *  > If local load is greater than the threshold, forward job to a random neigh
 *  > else, process it locally
 */
namespace fog {

    Define_Module(FogOracleForwarding);
    FogOracleForwarding::FogOracleForwarding() {
        // TODO Auto-generated constructor stub
    }

    FogOracleForwarding::~FogOracleForwarding() {
        // TODO Auto-generated destructor stub
    }

    void FogOracleForwarding::initialize() {
        FogLoadBalancer::initialize();
        if (FogLoadBalancer::loadOracle == nullptr){
            cRuntimeError *err = new cRuntimeError("No reference to loadOracle");
            throw(err);
        }
    }

    /**
     * Check if localLoad is less than thr
     */
    bool FogOracleForwarding::decideProcessLocally(FogJob *job) {
        EV<<"QUEUE: "<<queueCapacity<<endl;
        double localLoad=FogLoadBalancer::localLoad->getLowestLoad();
        EV<<"LOCAL LOAD: "<< localLoad <<"avf: "<< loadOracle->getAvgLoad()<<endl;
        if (localLoad < loadOracle->getAvgLoad()){
            return true;
        } else {
            return false;
        //return (localLoad < thr);
        }
    }

    /**
     * Never start probes
     */
    bool FogOracleForwarding::decideStartProbes(FogJob *job) {
        return false;
    }

    /**
     * Just one neigh
     */
    int FogOracleForwarding::getFanout() {
        //return every possible neighbor
        return probeGates.size();
    }


    double FogOracleForwarding::getLoadFromPort(int port){
        // get id from port number
        int moduleId=neighborMap->moduleIdFromPort(port);
        // get load from loadOracle
        double load = loadOracle->getLoad(moduleId);
        return load;
    }
    /**
     * Least loaded neighbor
     */
    int FogOracleForwarding::selectNeighbor(std::vector<int> neighbors) {
        //Select neighbor with lowest load according to load oracle
        // iterate over neighbors (full list of nodes)
        // for each neighbor get port and load
        double minLoad=getLoadFromPort(neighbors[0]);
        int port = neighbors[0];
        for (int i=0; i<neighbors.size(); i++){
            if (getLoadFromPort(neighbors[i])<minLoad){
                minLoad=getLoadFromPort(neighbors[i]);
                port=neighbors[i];
            }
        }
        return port;
    }

    bool FogOracleForwarding::decideForwardNow() {
        return false;
    }

}
