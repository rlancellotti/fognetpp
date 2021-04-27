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

#ifndef SRC_FOGPARALLELPROBING_H_
#define SRC_FOGPARALLELPROBING_H_

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

class FogParallelProbing: public FogLoadBalancer {
    public:
        FogParallelProbing();
        virtual ~FogParallelProbing();

        bool decideProcessLocally(FogJob *job);
        void initialize();

        bool decideStartProbes(FogJob *job);
        int getFanout();
        bool decideForwardNow(FogJob *job, int queryId);
    private:
        int fanout;
        int max_hop;
        int thr;
        int thr_shortcut;
    };

} /* namespace fog */

#endif /* SRC_FOGPARALLELPROBING_H_ */
