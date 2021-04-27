#include <iostream>
#include "ProbeQueue.h"

namespace fog {

    int ProbeQueue::nextProbeId = 0; // needed for using this static member
    int ProbeData::count =0; //counter for randomizing use of probe data
    // contructor
    ProbeData::ProbeData(std::vector<int> neighPorts, int jobId, int probeId, FogJob* job){
        this->neighPorts = neighPorts;
        this->neighLoads.resize(this->neighPorts.size(),-1);
        this->jobId = jobId;
        this->probeId = probeId;
        this->job = job;
        this->complete = false;
    }

    // retrieve Job
    FogJob * ProbeData::getJob(){ return this->job; }

    // get jobId
    const int ProbeData::getJobId(){ return this->jobId; }

    // get probeId
    const int ProbeData::getProbeId(){ return this->probeId; }

    // add response data in prove data structure
    void ProbeData::gotResponse(int neighPort, double load){
        // iterate over neighs to find the appropriate neighbor
        int neigh = -1;
        for (unsigned int i = 0; i < this->neighPorts.size(); i++) {
            if (this->neighPorts[i] == neighPort) {
                neigh = i;
                break;
            }
        }
        if (neigh<0) { return; }
        // set load
        // FIXME: handle exceptions
        this->neighLoads[neigh]=load;
        // check if it probe is complete
        if (! this->complete) {
            this->complete=true;
            for (unsigned int i = 0; i < this->neighLoads.size(); i++) {
                if (this->neighLoads[i] <0) {
                    this->complete=false;
                    break;
                }
            }
        }
    }

    // returns true if every query received the response
    const bool ProbeData::isComplete(){ return this->complete; }

    // Forces the query to be considered as complete
    void ProbeData::forceComplete(){ this->complete=true; }

    // returns value of  lowest load
    const double ProbeData::getLowestLoad(){
        double min = this->neighLoads[0];
        // FIXME: check operator precedence
        for (unsigned int i = 1; i < this->neighLoads.size(); i++) {
            if ((min <0) || ((min > this->neighLoads[i]) && (this->neighLoads[i] >= 0))) {
                min = this->neighLoads[i];
            }
        }
        return min;
    }

    // returns port of neigh with lowest load
    /*const int ProbeData::getLeastLoadedNeigh(){
        int min = this->neighLoads[0];
        int idx = 0;
        for (unsigned int i = 1; i < this->neighLoads.size(); i++) {
            if ((min <0) || ((min > this->neighLoads[i]) && (this->neighLoads[i] >= 0))) {
                min = this->neighLoads[i];
                idx = i;
            }
        }
        return this->neighPorts[idx];
    }*/
    const int ProbeData::getLeastLoadedNeigh(){
        double targetLoad = this->getLowestLoad();
        int idx = 0;
        for (unsigned int i = 0; i < this->neighLoads.size(); i++) {
            idx = (ProbeData::count + i) % this->neighLoads.size();
            if ((this->neighLoads[i] == targetLoad) && (this->neighLoads[i]>=0)) {
                break;
            }
        }
        ProbeData::count++;
        return this->neighPorts[idx];
    }



    //Giving a jobid, return its probe queue
    ProbeData* ProbeQueue::getDataFromJobId(int jobId){
        PDMap::iterator it = this->probesByJobId.find(jobId);
        if (it == this->probesByJobId.end()){
            return nullptr;
        } else {
            return it->second;
        }
    }

    //Giving a probeid, return its probe queue
    ProbeData* ProbeQueue::getDataFromProbeId(int probeId){
        PDMap::iterator it = this->probesByProbeId.find(probeId);
        if (it == this->probesByProbeId.end()){
            return nullptr;
        } else {
            return it->second;
        }
    }

    // create e new probe
    int ProbeQueue::addProbe(std::vector<int> neighs, int jobId, FogJob* job){
        int probeId = ProbeQueue::nextProbeId;
        ProbeData *pd = new ProbeData(neighs, jobId, probeId, job);
        // FIXME: check exception
        this -> probesByProbeId[probeId] = pd;
        this -> probesByJobId[jobId] = pd;
        ProbeQueue::nextProbeId ++;
        return probeId;
    }

    // Giving a jobid, deletes its probe queue
    void ProbeQueue::deleteFromJobId(int jobId){
        // find probeData
        PDMap::iterator itJ = this->probesByJobId.find(jobId);
        if (itJ != this->probesByJobId.end()){
            ProbeData *pd = itJ->second;
            int probeId = pd->getProbeId();
            this->probesByJobId.erase(itJ->first);
            // remove also from other map
            PDMap::iterator itP = this->probesByProbeId.find(probeId);
            if (itP != this->probesByProbeId.end()){
                this->probesByProbeId.erase(itP->first);
            }
            delete pd;
        }
    }

    // Giving a probeid, deletes its probe queue
    void ProbeQueue::deleteFromProbeId(int probeId){
        // find probeData
        PDMap::iterator itP = this->probesByProbeId.find(probeId);
        if (itP != this->probesByProbeId.end()){
            ProbeData *pd = itP->second;
            int jobId = pd->getJobId();
            this->probesByProbeId.erase(itP->first);
            // remove also from other map
            PDMap::iterator itJ = this->probesByJobId.find(jobId);
            if (itJ != this->probesByJobId.end()){
                this->probesByJobId.erase(itJ->first);
            }
            delete pd;
        }

    }

} // namespace


/*void checkProbeData(){
    std::vector<int> nPorts{1, 3, 2};
    int jobId=0; 
    fog::FogJob job = 'j';
    fog::ProbeData *pd = new fog::ProbeData(nPorts, jobId, 0, &job);
    std::cout << "Lowest load = " << pd->getLowestLoad() <<"\n";
    std::cout << "Least Loaded neighbor = " << pd->getLeastLoadedNeigh() <<"\n";
    std::cout << "Is Complete = " << (pd->isComplete()?"true":"false") << "\n";
    pd->gotResponse(3, 1);
    std::cout << "gotResponse(3,1)\n";
    std::cout << "Lowest load = " << pd->getLowestLoad() <<"\n";
    std::cout << "Least Loaded neighbor = " << pd->getLeastLoadedNeigh() <<"\n";
    std::cout << "Is Complete = " << (pd->isComplete()?"true":"false") << "\n";
    pd->gotResponse(2, 0);
    std::cout << "gotResponse(2,0)\n";
    std::cout << "Lowest load = " << pd->getLowestLoad() <<"\n";
    std::cout << "Least Loaded neighbor = " << pd->getLeastLoadedNeigh() <<"\n";
    std::cout << "Is Complete = " << (pd->isComplete()?"true":"false") << "\n";
    pd->gotResponse(2, 5);
    std::cout << "gotResponse(2,5)\n";
    std::cout << "Lowest load = " << pd->getLowestLoad() <<"\n";
    std::cout << "Least Loaded neighbor = " << pd->getLeastLoadedNeigh() <<"\n";
    std::cout << "Is Complete = " << (pd->isComplete()?"true":"false") << "\n";
    pd->gotResponse(1, 1);
    std::cout << "gotResponse(1,1)\n";
    std::cout << "Lowest load = " << pd->getLowestLoad() <<"\n";
    std::cout << "Least Loaded neighbor = " << pd->getLeastLoadedNeigh() <<"\n";
    std::cout << "Is Complete = " << (pd->isComplete()?"true":"false") << "\n";
    delete pd;
}

void  checkProbeQueue(){
    fog::FogJob j1 = 'j';
    fog::FogJob j2 = 'k';
    fog::ProbeQueue * pq = new fog::ProbeQueue();
    pq->addProbe(std::vector<int> {1,3,2}, 1, &j1); 
    pq->addProbe(std::vector<int> {4,2,5}, 5, &j2); 
    fog::ProbeData *pd1 = pq->getDataFromJobId(1);
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(2) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(0) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(5) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(0) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(2) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(3) << "\n";
    std::cout << "Job =" << (*(pd1->getJob())) << "\n";
    std::cout << "JobId =" << pd1->getJobId() << "\n";
    std::cout << "ProbeId =" << pd1->getProbeId() << "\n";
    fog::ProbeData *pd2 = pq->getDataFromJobId(5);
    std::cout << "Job =" << (*(pd2->getJob())) << "\n";
    std::cout << "JobId =" << pd2->getJobId() << "\n";
    std::cout << "ProbeId =" << pd2->getProbeId() << "\n";
    // delete by jobId
    pq->deleteFromJobId(5);
    // double delete to see if it works
    pq->deleteFromJobId(5);
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(5) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(0) << "\n";
    pq->deleteFromProbeId(0);
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(5) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromJobId(1) << "\n";
    std::cout << "Retrieved ps* =" << (void *) pq->getDataFromProbeId(0) << "\n";
    delete pq;
}

# define N 10000
void  stressProbeQueue(){
    fog::FogJob j1 = 'j';
    fog::FogJob j2 = 'k';
    fog::ProbeQueue * pq = new fog::ProbeQueue();
    int pid;
    for (int i=0; i< N; i++) {
        pid = pq->addProbe(std::vector<int> {1,3,2}, i, nullptr);
    }
    std::cout << "Allocated " << N << " probes, last probeId is " << pid << "\n";
    for (int i=pid +1 -N; i< pid + 1; i++) {
        pq->deleteFromProbeId(i);
    }
    std::cout << "Deallocated (from ProbeId) " << N << " probes\n";
    for (int i=0; i< N; i++) {
        pid = pq->addProbe(std::vector<int> {1,3,2}, i, &j1);
    }
    std::cout << "Allocated " << N << " probes, last probeId is " << pid << "\n";
    for (int i=0; i< N; i++) {
        pq->deleteFromJobId(i);
    }
    std::cout << "Deallocated (from JobId) " << N << " probes\n";
    delete pq;
}

int main(){
    checkProbeData();
    checkProbeQueue();
    stressProbeQueue();
}
*/
