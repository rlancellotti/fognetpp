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

#ifndef __FOGSOURCE_H
#define __FOGSOURCE_H

#include <omnetpp.h>
using namespace omnetpp;


namespace fog {

/**
 * Generates messages; see NED file for more info.
 */
class FogSource : public cSimpleModule
{
  private:
    cMessage *timerMessage;
    simtime_t maxInterval;
    simsignal_t sentJobSignal;
    //static int nextJobId;

  public:
     FogSource();
     virtual ~FogSource();
     static const char *getJobName();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}; // namespace

#endif
