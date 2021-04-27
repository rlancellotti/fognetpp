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

#ifndef FOGORACLEFORWARDING_H_
#define FOGORACLEFORWARDING_H_

#include <algorithm>
#include <omnetpp.h>
#include "../../queueinglib/QueueingDefs.h"
#include <omnetpp/cmsgpar.h>
#include <omnetpp/cqueue.h>
#include "FogLoadBalancer.h"
#include "FogJob_m.h"
#include "LoadUpdate_m.h"

using namespace omnetpp;

namespace fog {
    class FogOracleForwarding: public FogLoadBalancer {
        public:
            FogOracleForwarding();
            virtual ~FogOracleForwarding();
            void initialize();
            bool decideProcessLocally(FogJob *job); // decide if job have to be processed locally
            bool decideStartProbes(FogJob *job);    // decide if start probes
            int getFanout();
            int selectNeighbor(std::vector<int> neighbors); // select neigh to forward job
            bool decideForwardNow();

        protected:
            double getLoadFromPort(int port);

        };
}

#endif /* FOGORACLEFORWARDING_H_ */
