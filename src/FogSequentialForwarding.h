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

#ifndef FOGSEQUENTIALFORWARDING_H_
#define FOGSEQUENTIALFORWARDING_H_

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
    class FogSequentialForwarding: public FogLoadBalancer {
        public:
            int thr;
            bool adaptive;

            FogSequentialForwarding();
            virtual ~FogSequentialForwarding();
            void initialize();
            bool decideProcessLocally(FogJob *job); // decide if job have to be processed locally
            bool decideStartProbes(FogJob *job);    // decide if start probes
            int getFanout();
            int selectNeighbor(std::vector<int> neighbors); // select neigh to forward job
            bool decideForwardNow();

        protected:

        };
}

#endif /* FOGSEQUENTIALFORWARDING_H_ */
