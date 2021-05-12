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

#ifndef __FOG_LOADBALANCER_H_
#define __FOG_LOADBALANCER_H_

#include <omnetpp.h>
#include <string>
#include "msgTypes.h"
#include "LoadOracle.h"
#include "FogJob_m.h"
#include "ProbeAnswer_m.h"
#include "ProbeQuery_m.h"
#include "LoadUpdate_m.h"
#include "ProbeQueue.h"
#include "IdMessage_m.h"


using namespace omnetpp;

namespace fog {

class NeighborMap{
    public:
        void addNeighbor(int port, int moduleId);
        int moduleIdFromPort(int port);
        int portFromModuleId(int moduleId);
        void print(std::ostream& os);
    private:
        std::map<int, int>moduleIds;
        std::map<int, int>ports;
};

/**
 * Class to manage localLoad (load of local PU)
 */
class LocalLoad {
    public:

        int lastPu;             // last PU to which the job was sent
        std::vector<int> load;  // load for each pu

        LocalLoad(int nServers);

        //Update load of a PU
        void updateLoad(int pu, int newLoad);
        //Get least load PU
        int getLeastLoadedPU(bool readOnly);
        //Get least load
        int getLowestLoad();
        //Get a idle PU
        int getIdlePU();
};


/**
 * TODO - Generated class
 */
class FogLoadBalancer : public cSimpleModule
{

    public:
        FogLoadBalancer();
        virtual ~FogLoadBalancer();
        static const char *getAnswerName();
        static const char *getProbeQueryName();

    protected:
        int nServers;       //PU number
        int nApps;          // Appid number
        int queueCapacity;  // PU's queue capacity
        int max_hop;        // mazimum number of hops for a job

        LocalLoad *localLoad = nullptr;
        std::vector<int> probeGates;    // gates for neighs
        ProbeQueue pq;
        NeighborMap *neighborMap; // module ID <--> gate ID mapping
        LoadOracle * loadOracle = nullptr;
        bool bounceBack;
        void shuffle(std::vector<int> *);

        //Statics
        simsignal_t droppedSignal;
        simsignal_t loadSignal;
        simsignal_t jobQueryLoadErrorSignal;
        simsignal_t jobAbsQueryLoadErrorSignal;
        simsignal_t jobSignal;
        simsignal_t jobFromSrcSignal;
        simsignal_t jobFromNeighborSignal;
        simsignal_t jobLocalSignal;
        simsignal_t jobForwardSignal;
        simsignal_t probeQuerySignal;
        simsignal_t probeAnswerSignal;
        simsignal_t bounceBackSignal;
        simsignal_t jobForwardPortSignal;

        // per class
        /*std::map<int, int> nJobsPerClass;       // total number of jobs
        std::map<int, int> nLocalJobsPerClass;  // total number of jobs sent to local PU
        std::map<int, int> nRemoteJobsPerClass; // total number of jobs forwarded to another LB
        std::map<int, int> nDroppedJobsSLAPerClass; // number of jobs dropped due to SLA violations
        std::map<int, int> nProbesPerClass;     // number of probes started
        std::map<int, int> nProbeQueryPerClass; // number of probes messages sent
        std::map<int, int> nProbeAnswersPerClass;   // number of answers received

        int nJobs; // total number of jobs
        int nInternalJobs; // total number of jobs arrived from source
        int nExternalJobs; // total number of jobs arrived from another LB
        int nDroppedJobsSLA; // number of jobs due to SLA violations
        int nLocalJobs; // number of jobs sent to the local PU
        int nRemoteJobs; // number of jobs forwarded to a remote LB
        int nProbes; // number of probe started
        int nProbeQuery; // number of probe messages sent
        int nProbeAnswers; // number of useful probe answers received
        */

        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        virtual void processJob(FogJob *job);   // if incoming msg is a job, this function process it
        virtual void processProbeAnswer(ProbeAnswer *answer);      // if incoming msg is a probeAnswer, this function process it
        virtual void processProbeQuery(ProbeQuery *probeQuery); // if incoming msg is a probe request

        void handleLoadUpdate(LoadUpdate *loadUpdate);  // when arrive a local load update

        virtual bool decideProcessLocally(FogJob *job); // decide if job have to be processed locally
        virtual bool decideForwardNow(FogJob* job, int queryId);    // decide if job have to be forward now, so without wait other probes
        virtual bool decideStartProbes(FogJob *job);    // decide if start probing

        void startProbes(FogJob *job);
        void processLocally(FogJob *job);
        virtual void forwardJob(FogJob *job, char *gateName, int gateIndex); // forward job at the gateName port
        void forwardFromProbe(int queryId);
        void bounceBackJob(FogJob *job);

        virtual int selectNeighbor(std::vector<int> neighbors); // select neighbor to send job based on probing
        virtual int selectLocalPU(); // select localPU which going to process job

        virtual bool checkSlaExpired(FogJob *job);   // check if job is to be dropped due SLA violation
        virtual std::vector<int> getNeighbors(int fanout);  // get list of neighbors to send probing
        virtual int getFanout(); // get fanout
        int getNumServers();    // get number of servers in this Fog node
        int getSource(FogJob *job); // says if arrived job is from internal (source) or from external (other LB)
        // management of initial message with module ID
        void sendId();
        void processIdMsg(IdMessage *msg);


        //void dumpStat(std::map<int, int> *map, std::string name);
        virtual void finish();


};

} //namespace

#endif
