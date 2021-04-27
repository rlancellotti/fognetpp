#include "FogMerge.h"

namespace fog {

Define_Module(FogMerge);

void FogMerge::handleMessage(cMessage *msg)
{
    send(msg, "out");
}

}; //namespace

