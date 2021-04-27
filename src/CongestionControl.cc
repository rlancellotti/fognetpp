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

#include "CongestionControl.h"
#include "CongestionUpdate_m.h"

namespace fog {

Define_Module(CongestionControl);

CongestionControl::CongestionControl()
{
    timerMessage = NULL;
}

CongestionControl::~CongestionControl()
{
    cancelAndDelete(timerMessage);
}

void CongestionControl::initialize()
{
    timerMessage = new cMessage("timer");
    congestion=false;
    scheduleAt(simTime(), timerMessage);
}

void CongestionControl::handleMessage(cMessage *msg)
{
    ASSERT(msg==timerMessage);
    simtime_t t, trand;
    int ngates, i;
    // create new message
    CongestionUpdate *congMsg= new CongestionUpdate("CongUpdate");
    congMsg->setCongestion(congestion);
    if (congestion) {
        congMsg->setMultiplier(par("congestionMultiplier").doubleValue());
    } else {
        congMsg->setMultiplier(1.0);
    }
    ngates=gateSize("congestionControl");
    for (i=ngates-1; i>=0; i--){
        if (i==0) {
            send(congMsg, "congestionControl", i);
        } else {
            send(congMsg->dup(), "congestionControl", i);
        }
    }
    // schedule next message
    if (congestion){
        trand=par("fromCongestionStatusInterval").doubleValue();
    } else {
        trand=par("toCongestionStatusInterval").doubleValue();
    }
    //toggle congestion status for next message
    congestion=!congestion;
    // if trand<0 we disable the change of status
    if (trand>=0){
        t=simTime() + trand;
        scheduleAt(t, timerMessage);
    }
}

}; // namespace
