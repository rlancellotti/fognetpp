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
#include "FogSequentialForwarding.h"
#include "FogJob_m.h"

/**
 * Sequential Forward:
 *  > If local load is greater than the threshold, forward job to a random neigh
 *  > else, process it locally
 */
namespace fog {

    Define_Module(FogSequentialForwarding);
    FogSequentialForwarding::FogSequentialForwarding() {
        // TODO Auto-generated constructor stub
    }

    FogSequentialForwarding::~FogSequentialForwarding() {
        // TODO Auto-generated destructor stub
    }

    void FogSequentialForwarding::initialize() {
        FogLoadBalancer::initialize();
        thr = par("thr");
        adaptive = par("adaptive");
    }

    /**
     * Check if localLoad is less than thr
     */
    bool FogSequentialForwarding::decideProcessLocally(FogJob *job) {
        int count = job->getBalancerCount();
        // TODO: Prendi il valore a modo
        //EV<<"leastLoad: "<<localLoad->getLeastLoad()<<endl;
        EV<<"ADAPTATIVE: "<<adaptive<<endl;
        EV<<"QUEUE: "<<queueCapacity<<endl;
        float localLoad=FogLoadBalancer::localLoad->getLowestLoad();
        if (adaptive) {
            int thr_ad = (int)( (job->getBalancerCount()+1)*(((float)queueCapacity))/(float)max_hop ) ;
            EV<<"COUNT: "<<count<<" MAX_HOP: "<<max_hop<<" THR-AD: "<<thr_ad<<" Queue_capacity: "<<queueCapacity;
            EV<<"LOCAL LOAD: "<<localLoad<<endl;
            return (localLoad < thr_ad);
        }
        else {
            EV<<"LOCAL LOAD: "<< localLoad <<"thr: "<< thr<<endl;
            if (localLoad < thr || thr <0){
                return true;
            } else {
                return false;
            }
            //return (localLoad < thr);
        }
    }

    /**
     * Never start probes
     */
    bool FogSequentialForwarding::decideStartProbes(FogJob *job) {
        return false;
    }

    /**
     * Just one neigh
     */
    int FogSequentialForwarding::getFanout() {
        return 1;
    }

    /**
     * A random neigh
     */
    int FogSequentialForwarding::selectNeighbor(std::vector<int> neighbors) {
        // No need to randomize: neighbor is an already-randomized subset of the global neighbor list
        // neighbor should have a size of <fanout> that is equal to 1
        EV<<"Neigh: "<<neighbors.at(0);
        return (neighbors.at(0));
    }

    bool FogSequentialForwarding::decideForwardNow() {
        return false;
    }

}
