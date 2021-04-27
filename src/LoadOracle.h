#ifndef FOG_LOADORACLE_H_
#define FOG_LOADORACLE_H_

#include <vector>
#include <map>
#include <omnetpp.h>

using namespace omnetpp;

namespace fog {
class LoadRecord {
    private:
        double load;
        simtime_t lastSeen;
    public:
        LoadRecord(double load, simtime_t lastSeen);
        double getLoad();
        bool isStale(simtime_t delta);
};

typedef std::map<int, LoadRecord *> loadtable_t;

class LoadTable {
    private:
        loadtable_t table;
    public:
        LoadTable();
        void recordLoad(int nodeId, double load);
        void recordLoad(int nodeId, double load, simtime_t time);
        LoadRecord *getRecord(int nodeId);
        double getLoad(int nodeId);
        double getAvgLoad();
        bool isLoadStale(int nodeId, simtime_t delta);
};

class LoadOracle : public cSimpleModule{
    private:
        LoadTable * loadTable;
    protected:
        void initialize();
        void handleMessage(cMessage *msg);
    public:
        double getLoad(int nodeId);
        double getAvgLoad();
};

} // namespace
#endif // FOG_LOADORACLE_H_
