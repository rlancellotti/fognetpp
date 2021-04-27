#ifndef __FOG_PROBEQUEUE_H_
#define __FOG_PROBEQUEUE_H_

#include <vector>
#include <map>

#include "FogJob_m.h"

namespace fog {

// FIXME: this is a dirty hack
//typedef char FogJob;

class ProbeData {
    private:
        FogJob* job;    // enqueued job
        int jobId;      // job id of enqueued job
        int probeId;    // probe id of the enqueued
        bool complete;

        std::vector<int> neighPorts;      // list of ports corresponding to the neighs to which the probe has been sent
        std::vector<double> neighLoads;   //Neighs' answers, these are mapped with neighs, so load[0]->neigs[0]

        static int count;

    public:
        // contructor
        ProbeData(std::vector<int> neighPorts, int jobId, int probeId, FogJob* job);
        // add response data in prove data structure
        void gotResponse(int neighPort, double load);
        // retrieve Job
        FogJob * getJob();
        // get jobId
        const int getJobId();
        // get probeId
        const int getProbeId();
        // returns true if every query received the response
        const bool isComplete();
        // Forces the query to be considered as complete
        void forceComplete();
        // returns value of  lowest load
        const double getLowestLoad();
        // returns port of neigh with lowest load
        const int getLeastLoadedNeigh();
};

typedef std::map<int, ProbeData*> PDMap;

/**
 * Class to manage probe queue
 */
class ProbeQueue {
    public:
        static int nextProbeId; // auto increment id for enqueued query
        //static void addQuery(, std::map<int, ProbeQueue*> probesQueue, int jobId, FogJob* job);
        // Giving a jobid, return its probe queue
        ProbeData* getDataFromJobId(int jobId);
        // Giving a probeid, return its probe queue
        ProbeData* getDataFromProbeId(int probeId);
        // create a new probe: returns probeId
        int addProbe(std::vector<int> neighs, int jobId, FogJob* job);
        // Giving a jobid, deletes its probe queue
        void deleteFromJobId(int jobId);
        // Giving a probeid, deletes its probe queue
        void deleteFromProbeId(int probeId);
    private: 
        PDMap probesByProbeId;
        PDMap probesByJobId;
};

} // namespace
#endif
