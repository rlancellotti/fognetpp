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

#include "FogParallelProbing.h"

namespace fog {

    Define_Module(FogParallelProbing);

    FogParallelProbing::FogParallelProbing() {
        // TODO Auto-generated constructor stub

    }

    FogParallelProbing::~FogParallelProbing() {
        // TODO Auto-generated destructor stub
    }

    void FogParallelProbing::initialize() {
        FogLoadBalancer::initialize();
        fanout = par("fanout");
        thr = par("thr");
        thr_shortcut = par("thr_shortcut");
    }

    bool FogParallelProbing::decideProcessLocally(FogJob *job) {
        EV << "FogParallelProbing::decideProcessLocally(" << job->getId() << ")\n";
        float lowestLocalLoad=FogLoadBalancer::localLoad->getLowestLoad();
        ProbeData * pd = pq.getDataFromJobId(job->getId());
        if (pd == nullptr) { // no probing being carried out
            EV << "localLoad: " << lowestLocalLoad << " thr: " << thr << "\n";
            return (lowestLocalLoad < thr);
        } else { // probing is underway
            int lowestRemoteLoad = pd->getLowestLoad();
            EV << "localLoad: " << lowestLocalLoad << " remoteLoad: " <<lowestRemoteLoad << "\n";
            return (lowestRemoteLoad > lowestLocalLoad);
        }
    }

    bool FogParallelProbing::decideStartProbes(FogJob *job) {
        return true;
    }

    int FogParallelProbing::getFanout() {
        return fanout;
    }

    bool FogParallelProbing::decideForwardNow(FogJob *job, int queryId) {
        EV << "FogParallelProbing::decideForwardNow(" << job->getId() << ")\n";
        ProbeData * pd = pq.getDataFromProbeId(queryId);
        int remoteLoad = pd->getLowestLoad();
        EV << "remoteLoad: " << remoteLoad << " thr_shortcut: " << thr_shortcut << "\n";
        return (remoteLoad < thr_shortcut);
    }


} /* namespace fog */
