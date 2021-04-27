#include "LoadOracle.h"
#include "LoadUpdate_m.h"
namespace fog{

Define_Module(LoadOracle);

LoadRecord::LoadRecord(double l, simtime_t ls){
    this->load=l;
    this->lastSeen=ls;
}

double LoadRecord::getLoad(){return this->load;}

bool LoadRecord::isStale(simtime_t delta){
    return (this->lastSeen+delta >= simTime());
}


LoadTable::LoadTable(){}

void LoadTable::recordLoad(int nodeId, double load, simtime_t time){
    loadtable_t::const_iterator it = this->table.find(nodeId);
    if (it==this->table.end()){
        this->table.insert({nodeId, new LoadRecord(load, time)});
    } else {
        LoadRecord *oldrec = table.at(nodeId);
        this->table.at(nodeId) = new LoadRecord(load, time);
        delete(oldrec);
    }
}

void LoadTable::recordLoad(int nodeId, double load){
    recordLoad(nodeId, load, simTime());
}


LoadRecord * LoadTable::getRecord(int nodeId){
    loadtable_t::const_iterator it = this->table.find(nodeId);
    if (it==this->table.end()){
        return nullptr;
    } else {
        return it->second;
    }
}

double LoadTable::getLoad(int nodeId){
    LoadRecord *rec = this->getRecord(nodeId);
    if (rec != nullptr){
        return rec->getLoad();
    } else {
        return -1;
    }
}

double LoadTable::getAvgLoad(){
    if(!this->table.empty()){
        loadtable_t::iterator it;
        double load=0.0;
        for (it = table.begin(); it != table.end(); it++){
            load +=it->second->getLoad();
        }
        return load/table.size();
    } else {
        return 0.0;
    }
}

bool LoadTable::isLoadStale(int nodeId, simtime_t delta){
    LoadRecord *rec = this->getRecord(nodeId);
    if (rec != nullptr){
        return rec->isStale(delta);
    } else {
        return true;
    }
}

void LoadOracle::initialize(){
    this ->loadTable = new LoadTable();
    EV<<"AvgLoad: "<<this->loadTable->getAvgLoad()<<endl;
}

void LoadOracle::handleMessage(cMessage *msg){
    if (strcmp(msg->getName(), "loadUpdate") == 0) {
        LoadUpdate *loadUpdate = check_and_cast<LoadUpdate *>(msg);
        int moduleId = loadUpdate->getSenderModuleId();
        loadTable->recordLoad(moduleId, loadUpdate->getLoad());
        EV<< "LoadOracle: updateMessage ("<<moduleId<<", "<<loadUpdate->getLoad()<<")"<<endl;
        delete msg;
    }
}

double LoadOracle::getLoad(int nodeId){
    Enter_Method("getLoad()");
    return this->loadTable->getLoad(nodeId);
}

double LoadOracle::getAvgLoad(){
    //return 0.0;
    Enter_Method("getAvgLoad()");
    return this->loadTable->getAvgLoad();
}


}//namespace
