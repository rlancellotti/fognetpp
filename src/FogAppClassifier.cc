#include "FogAppClassifier.h"
#include "FogJob_m.h"

namespace fog {

Define_Module(FogAppClassifier);

void FogAppClassifier::handleMessage(cMessage *msg)
{
    FogJob *job = check_and_cast<FogJob *>(msg);
    int outGateIndex = job->getAppId()-1;
    EV << "sendng job to gate " << outGateIndex << "\n";
    if (outGateIndex < 0 || outGateIndex >= gateSize("out"))
        if(gate("rest")->isConnected()){
            send(job, "rest");
        } else {
            delete job;
        }
    else
        send(job, "out", outGateIndex);
}

}; //namespace

