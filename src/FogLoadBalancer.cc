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

#include <algorithm>
#include <string>
#include "LoadUpdate_m.h"
#include "FogLoadBalancer.h"
#include "FogJob_m.h"
#include "FogPU.h"
#include "ProbeAnswer_m.h"
#include "ProbeQuery_m.h"
#include "IdMessage_m.h"

namespace fog {

    Define_Module(FogLoadBalancer);
    const int SRC_SENSOR = 0;
    const int SRC_NEIGHBOR = 1;

    LocalLoad::LocalLoad(int nServers) {
        load.resize(nServers, 0);
        load.reserve(nServers);
        lastPu = 0;
    }

    /**
     * Set new load of the pu
     */
    void LocalLoad::updateLoad(int pu, int newLoad) {
        load[pu] = newLoad;
    }

    /**
     * Get least load PU
     * @param readOnly: if True, lastPU is not increased
     */
    int LocalLoad::getLeastLoadedPU(bool readOnly) {
        int min = load[lastPu];
        int minIndex = lastPu;
        for (unsigned int i = lastPu+1; i < load.size()+lastPu; i++) {
            if (min > load.at(i%load.size())) {
                minIndex = i%load.size();
                min = load.at(i%load.size());
            }
        }
        if (!readOnly)
            lastPu = (minIndex+1)%load.size();
        return minIndex;
    }

    /**
     * Get least load value.
     */
    int LocalLoad::getLowestLoad() {
        int min = load.at(0);
        for (unsigned int i = 1; i < load.size(); i++) {
            if (min > load.at(i)) {
                min = load.at(i);
            }
        }
        return min;
    }

    /**
     * Get first pu in idle
     */
    int LocalLoad::getIdlePU() {
        for (unsigned int i = 0; i < load.size(); i++)
            if (load.at(i) == 0)
                return i;
        return -1;
    }

    //------------------------------------------------------------

    void NeighborMap::addNeighbor(int port, int moduleId){
        moduleIds[port]=moduleId;
        ports[moduleId]=port;
    }

    int NeighborMap::moduleIdFromPort(int port){return moduleIds[port];}

    int NeighborMap::portFromModuleId(int moduleId){return ports[moduleId];}

    void NeighborMap::print(std::ostream &os){
        for(std::map<int,int>::iterator iter = moduleIds.begin(); iter != moduleIds.end(); ++iter)
        {
            int port =  iter->first;
            os << "port("<<port<<")->module("<< moduleIds[port]<<");\tmodule("<<moduleIds[port]<<")->port("<<ports[moduleIds[port]]<<")"<<endl;
        }
    }


    //------------------------------------------------------------

    FogLoadBalancer::FogLoadBalancer() {
        // TODO Auto-generated constructor stub
    }

    FogLoadBalancer::~FogLoadBalancer() {
    }

    /**
     * Initialize variables
     */
    void FogLoadBalancer::initialize()
        {
            nServers = getNumServers();
            localLoad = new LocalLoad(nServers);
            neighborMap = new NeighborMap();
            bounceBack = par("bounceBack");
            max_hop = par("max_hop");
            queueCapacity = par("queue");

            // Get neighs number and fill probeGates
            int probeGateSize = gateSize("loadToNeighbor");
            probeGates.resize(probeGateSize);
            for (int i = 0; i < probeGateSize; i++){probeGates[i] = i;}

            /*nJobs=0;
            nInternalJobs=0;
            nExternalJobs=0;
            nDroppedJobsSLA=0;
            nLocalJobs=0;
            nRemoteJobs=0;
            nProbes=0;
            nProbeQuery=0;
            nProbeAnswers=0;*/

            cModule * mod = getModuleByPath(par("loadOracleName").stringValue());
            EV<< "getModuleByPath(\"" << par("loadOracleName").stringValue() << "\")->" << mod<<endl;
            if (mod !=nullptr){loadOracle = check_and_cast<LoadOracle*>(mod);}
            EV<<"LoadOracle ptr is: " <<(void *) loadOracle<<endl;

            // signals
            droppedSignal = registerSignal("dropped");
            loadSignal = registerSignal("load");
            emit(loadSignal, 0.0);
            jobSignal = registerSignal("job");
            jobFromSrcSignal = registerSignal("jobFromSrc");
            jobFromNeighborSignal = registerSignal("jobFromNeighbor");
            jobForwardSignal = registerSignal("jobForward");
            jobLocalSignal = registerSignal("jobLocal");
            probeQuerySignal = registerSignal("probeQuery");
            probeAnswerSignal = registerSignal("probeAnswer");
            bounceBackSignal = registerSignal("bouncedBack");
            jobForwardPortSignal = registerSignal("jobForwardPort");
            jobQueryLoadErrorSignal = registerSignal("jobQueryLoadError");
            jobAbsQueryLoadErrorSignal = registerSignal("jobAbsQueryLoadError");
            sendId();
        }

    void FogLoadBalancer::sendId(){
        // get module ID
        int id=this->getId();
        int nports = this->gateSize("loadToNeighbor");
        for (int i=0; i<nports; i++) {
            // create message(s)
            IdMessage *msg = new IdMessage("idMessage", KIND_OTHER);
            msg->setModuleId(id);
            // send messages to all neighbors
            send(msg, "loadToNeighbor", i);
            //EV<<"Sending id: " << id << "to neighbor at gate: " << i <<endl;
        }
    }

    void FogLoadBalancer::processIdMsg(IdMessage *msg){
        //EV<<"Got ID message from: " << msg->getModuleId() << " port " << msg->getArrivalGate()->getName() << "[" << msg->getArrivalGate()->getIndex() << "]" <<endl;
        neighborMap -> addNeighbor(msg->getArrivalGate()->getIndex(), msg->getModuleId());
        //nm ->print(EV);
        delete(msg);
    }

    /**
     * Handle incoming message.
     * They can be:
     *  > job
     *  > loadUpdate
     *  > probeQuery
     *  > probeAnswer
     *  > idMessage
     */
    void FogLoadBalancer::handleMessage(cMessage *msg)
        {
            //In case incoming msg is a job
            if (strcmp(msg->getName(), "job") == 0) {
                FogJob *job = check_and_cast<FogJob *>(msg);
                EV<<"Me: "<<this->getId()<<" received job from: "<<job->getArrivalGate()->getName()<<"["<<job->getArrivalGate()->getIndex()<<"]"<<endl;
                job->setBalancerCount(job->getBalancerCount()+1);
                processJob(job);
            }
            //In case incoming msg is a loadUpdate
            else if (strcmp(msg->getName(), "loadUpdate") == 0) {
                LoadUpdate *loadUpdate = check_and_cast<LoadUpdate *>(msg);
                handleLoadUpdate(loadUpdate);
                delete msg;
            }
            //In case incoming msg is a probeQuery
            else if (strcmp(msg->getName(), getProbeQueryName()) == 0) {
                ProbeQuery *probeQuery = check_and_cast<ProbeQuery *>(msg);
                processProbeQuery(probeQuery);
                delete msg;
            }
            //In case incoming msg is a probeAnswer
            else if (strcmp(msg->getName(), getAnswerName()) == 0) {
                ProbeAnswer *answer = check_and_cast<ProbeAnswer *>(msg);
                processProbeAnswer(answer);
                delete msg;
            }
            //In case incoming msg is a idMessage
            else if (strcmp(msg->getName(), "idMessage") == 0) {
                IdMessage *idmsg = check_and_cast<IdMessage *>(msg);
                processIdMsg(idmsg);
            }
        }

    /**
     * Process incoming job.
     *
     * If SLA is expired and job is flagged as real time -> drop it
     * Otherwise:
     *      -> If job is from another fog and multihop=false (forward not allowed), send job to a local pu
     *      -> If it can be forwarded, check if do it (or probing)
     */
    void FogLoadBalancer::processJob(FogJob *job) {
        emit(jobSignal, 1);
        int jobSrc = getSource(job);
        int count = job->getBalancerCount();
        if (jobSrc == SRC_SENSOR) {
            emit(jobFromSrcSignal, 1);
            //If job is from internal, initialize timeStamp to compute balancerTime
            job->setTimestamp();
        } else if (jobSrc == SRC_NEIGHBOR) {
            emit(jobFromNeighborSignal, 1);
            // log difference between expected and current load state
            if (job->getExpectedLoad()>0){
                float delta=(job->getExpectedLoad())-(localLoad->getLowestLoad());
                emit(jobQueryLoadErrorSignal, (double) delta);
                emit(jobAbsQueryLoadErrorSignal, (double) std::abs(delta));
            }
        }

        if (checkSlaExpired(job)) {
            emit(droppedSignal, 1);
            dropAndDelete(job);
            return;
        }
        //Check Max hop flag
        if (max_hop >= 0 && max_hop < count){
            processLocally(job);
            return;
        }
        // Check if we should bounce back the job
        if ((jobSrc == SRC_NEIGHBOR) && (bounceBack == true) && (job->getPreviousLoad() > 0) && (job->getPreviousLoad() < localLoad->getLowestLoad())) {
            bounceBackJob(job);
            return;
        }
        if (decideProcessLocally(job)) { // processing locally
            processLocally(job);
        } else { // do not processing locally
            if (decideStartProbes(job)) {
                startProbes(job);
            } else { // job to forward to another dispatcher
                std::vector<int> neighs = getNeighbors(getFanout());
                int neigh = selectNeighbor(neighs);
                emit(jobForwardPortSignal, neigh);
                forwardJob(job, (char *) "loadToNeighbor", neigh);
            }
        }
    }

    /**
     * Handle probe request.
     * Send my least local load.
     */
    void FogLoadBalancer::processProbeQuery(ProbeQuery *probeQuery) {
        int nGate = probeQuery->getArrivalGate()->getIndex();
        int load = localLoad->getLowestLoad();
        int queryId = probeQuery->getQueryId();
        int appId = probeQuery->getAppId();
        //nProbeAnswersPerClass[appId]++;
        //nProbeAnswers++;

        ProbeAnswer *answer = new ProbeAnswer(getAnswerName(), KIND_ANSWER);
        answer->setLoad(load);
        answer->setQueryId(queryId);
        answer->setAppId(appId);
        answer->setByteLength(par("probeAnswerPacketLength"));
        send(answer, "loadToNeighbor", nGate);
    }

    void FogLoadBalancer::forwardFromProbe(int queryId) {
        ProbeData *pd = pq.getDataFromProbeId(queryId);
        FogJob *job = pd->getJob();
        int neigh = pd->getLeastLoadedNeigh();
        job->setExpectedLoad(pd->getLowestLoad());
        emit(jobForwardPortSignal, neigh);
        forwardJob(job, (char *) "loadToNeighbor", neigh);
        pq.deleteFromProbeId(queryId);
    }

    /**
     * Handle probe answers
     * Fill probesQueue with answers.
     */
    void FogLoadBalancer::processProbeAnswer(ProbeAnswer *answer) {
        int fogId = answer->getArrivalGate()->getIndex();
        double load = answer->getLoad();
        int queryId = answer->getQueryId();
        //int appId = answer->getAppId();
        emit(probeAnswerSignal, 1);
        // FIXME: modify this code according to new classes
        //When query with id==queryId is no longer active it will be == null
        ProbeData *pd = pq.getDataFromProbeId(queryId);
        if (pd != nullptr) {
            // update load information
            pd->gotResponse(fogId, load);
            //If SLA is expired, delete job and its query
            FogJob *job = pd->getJob();
            if (checkSlaExpired(job)) {
                dropAndDelete(job);
                pq.deleteFromProbeId(queryId);
            } else { // SLA is ok
                if (decideForwardNow(job, queryId)) {
                    //forward to neigh without waiting for other probes
                    forwardFromProbe(queryId);
                } else { // Maybe we can wait. Check if this is the last probe
                    if (pd->isComplete()){
                        //We can't wait: this is last answer
                        if (decideProcessLocally(job)) {
                            processLocally(job);
                            pq.deleteFromProbeId(queryId);
                        } else {  //forward to leastload neigh
                            forwardFromProbe(queryId);
                        }
                    }
                }
            }
        }
    }

    bool FogLoadBalancer::decideProcessLocally(FogJob *job) {
        if (localLoad->getLowestLoad() == 0)
            return true;
        //if (other_rules)
        //    return true;
        return false;
    }

    /**
     * To redefine
     */
    bool FogLoadBalancer::decideStartProbes(FogJob *job) {
        return true;
    }

    /**
     * To redefine
     */
    bool FogLoadBalancer::decideForwardNow(FogJob *job, int queryId) {
        return false;
    }

    /**
     * Forward job to selected PU
     */
    void FogLoadBalancer::forwardJob(FogJob *job, char *gateName, int gateIndex) {
        //updateStats
        EV<<"Io: "<<this->getId()<<" invio job a: "<<gateName<<"["<<gateIndex<<"]"<<endl;
        job->setPreviousLoad(localLoad->getLowestLoad());
        emit(jobForwardSignal, 1);
        send(job, gateName, gateIndex);
    }

    /**
     * Return job to sender PU
     */
    void FogLoadBalancer::bounceBackJob(FogJob *job) {
        // if we are bouncing back, the number of hops is not increased.
        job->setBalancerCount(job->getBalancerCount()-1);
        //job->setPreviousLoad(localLoad->getLowestLoad());
        cGate *senderGate = job->getSenderGate();
        EV<<"Io: "<<this->getId()<<" bouncingBack from: "<< senderGate->getName()<<"["<<senderGate->getIndex()<<"]"<<endl;
        emit(bounceBackSignal, 1);
        send(job, "loadToNeighbor", senderGate->getIndex());
    }


    /**
     * Select the neighbor to send the job to
     */
    int FogLoadBalancer::selectNeighbor(std::vector<int> neighs) {
        return 0;
    }

    /**
     * Handle local load update.
     */
    void FogLoadBalancer::handleLoadUpdate(LoadUpdate *msg) {
        localLoad->updateLoad(msg->getArrivalGate()->getIndex(), msg->getLoad());
        double l = localLoad->getLowestLoad();
        emit(loadSignal, l);
        // FIXME: update loadOracle
        if (loadOracle!=nullptr){
            LoadUpdate *lu = new LoadUpdate("loadUpdate", KIND_LOAD);
            lu->setLoad(l);
            sendDirect(lu, loadOracle, "loadUpdate");
        }
    }

    int FogLoadBalancer::getFanout() {
        return 1;
    }

    /**
     * Return job's arrival gate (from external or internal)
     */
    int FogLoadBalancer::getSource(FogJob *job) {
        cGate *gate = job->getArrivalGate();
        if (strcmp(gate->getName(), "loadFromNeighbor") == 0) {
            return SRC_NEIGHBOR;
        }
        return SRC_SENSOR;
    }

    /**
     * Return how much servers there are
     */
    int FogLoadBalancer::getNumServers() {
        return gateSize("out");
    }

    void FogLoadBalancer::shuffle(std::vector<int> *v){
        for (int i=v->size()-1; i>=1; i--){
            int j=intrand(i+1); //0 <= j <= i
            int x=v->at(i);
            v->at(i)=v->at(j);
            v->at(j)=x;
        }
        // print vector
        EV<<"Shuffle: [";
        for (std::vector<int>::const_iterator i = v->begin(); i != v->end(); ++i)
            EV << *i << ", ";
        EV<< "]" << endl;
    }

    /**
     * Return a list of neighs to send probe to
     */
    std::vector<int> FogLoadBalancer::getNeighbors(int fanout) {
        //Randomize the vector containing all neighs and return subvector
        //according with fanout for size
        shuffle(&probeGates);
        //std::random_shuffle(probeGates.begin(), probeGates.end());
        std::vector<int> neighs;
        neighs.resize(fanout);
        for (int i = 0; i < fanout; i++)
            //neighs.insert(neighs.begin(), probeGates.at(i));
            neighs[i]=probeGates[i];
        EV<<"getNeighbor: [";
        for (std::vector<int>::const_iterator i = neighs.begin(); i != neighs.end(); ++i)
            EV << *i << ", ";
        EV<< "]" << endl;
        return neighs;
    }

    /**
     * Select local PU
     */
    int FogLoadBalancer::selectLocalPU() {
        return localLoad->getLeastLoadedPU(false);
    }

    /**
     * Return true if I have to drop job
     * So, if it is flag as realTime and if SLA is expired
     */
    bool FogLoadBalancer::checkSlaExpired(FogJob *job) {
        return (job->getRealTime() && !(simTime()<=job->getSlaDeadline()));
    }

    /**
     * Process job locally.
     * Select a local pu (with selectLocalPU() function) and send job to it
     */
    void FogLoadBalancer::processLocally(FogJob *job) {
        int selectPU = selectLocalPU();
        if (selectPU == -1) { selectPU = 0; }
        emit(jobLocalSignal, 1);
        //nLocalJobsPerClass[job->getAppId()]++;
        //nLocalJobs++;
        simtime_t now = simTime();
        simtime_t ts = job->getTimestamp();
        job->setBalancerTime(now-ts);
        forwardJob(job, (char *) "out", selectPU);
        //dropAndDelete(job);
    }

    /**
     * Start probes for this job.
     * Fill up probeQueue with a new query
     */
    void FogLoadBalancer::startProbes(FogJob *job) {
        int queryId, fanout = getFanout();
        std::vector<int> neighs = getNeighbors(fanout);
        EV<<"StartProbe([";
        for (unsigned int i=0; i<neighs.size(); i++){EV<<neighs[i]<<", ";}
        EV<<"])"<<endl;
        queryId = pq.addProbe(neighs, job->getId(), job);
        //nProbesPerClass[job->getAppId()]++;
        //nProbes++;
        emit(probeQuerySignal, (long) neighs.size());
        //Send to neighs a probe request
        for (unsigned int i = 0; i < neighs.size(); i++) {
            //nProbeQueryPerClass[job->getAppId()]++;
            //nProbeQuery++;
            ProbeQuery *pquery = new ProbeQuery(getProbeQueryName(), KIND_QUERY);
            pquery->setQueryId(queryId);
            pquery->setJobId(job->getId());
            pquery->setAppId(job->getAppId());
            pquery->setByteLength(par("probeQueryPacketLength"));
            send(pquery, (char *) "loadToNeighbor", neighs.at(i));
        }
    }

    /*void FogLoadBalancer::dumpStat(std::map<int, int> *map, std::string name){
        for(std::map<int, int>::iterator i = map->begin(); i != map->end(); ++i){
            int appId=i->first;
            int value=i->second;
            //std::string recname = name + "_appId: " + std::to_string(appId);
            std::string recname = name + "_" + std::to_string(appId);
            recordScalar(recname.c_str(), value);
        }
    }*/

    void FogLoadBalancer::finish(){
        /*recordScalar("#process locally",nLocalJobs);
        recordScalar("#process remote",nRemoteJobs);
        recordScalar("#sent probes",nProbeQuery);
        recordScalar("#sent answers",nProbeAnswers);

        dumpStat(&nJobsPerClass, "#jobs per class");
        dumpStat(&nRemoteJobsPerClass, "#process remote per class");
        dumpStat(&nLocalJobsPerClass, "#process locally per class");
        dumpStat(&nDroppedJobsSLAPerClass, "#dropped jobs sla per class");
        dumpStat(&nProbesPerClass, "#starting probes per class");
        dumpStat(&nProbeQueryPerClass, "#sent probes query per class");
        dumpStat(&nProbeAnswersPerClass, "#sent answers per class");*/

        /*recordScalar("localJobs",nLocalJobs);
        recordScalar("internalJobs",nInternalJobs);
        recordScalar("externalJobs",nExternalJobs);
        recordScalar("remoteJobs",nRemoteJobs);
        recordScalar("probeQueries",nProbeQuery);
        recordScalar("probeAnswers",nProbeAnswers);
        recordScalar("droppedJobsSLA", nDroppedJobsSLA);

        dumpStat(&nJobsPerClass, "jobs_class");
        dumpStat(&nRemoteJobsPerClass, "remoteJobs_class");
        dumpStat(&nLocalJobsPerClass, "localJobs_class");
        dumpStat(&nDroppedJobsSLAPerClass, "droppedJobsSLA_class");
        dumpStat(&nProbesPerClass, "probes_class");
        dumpStat(&nProbeQueryPerClass, "probeQueries_class");
        dumpStat(&nProbeAnswersPerClass, "probeAnswers_class");*/
    }

    const char *FogLoadBalancer::getAnswerName(){
        return "probeAnswer";
    }

    const char *FogLoadBalancer::getProbeQueryName(){
        return "probeQuery";
    }

} //namespace

