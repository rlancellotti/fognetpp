//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __FOG_CLASSIFIER_H
#define __FOG_CLASSIFIER_H

#include <omnetpp.h>

using namespace omnetpp;
namespace fog {

/**
 * See the NED declaration for more info.
 */
class  FogAppClassifier : public cSimpleModule
{
    private:
    protected:
        virtual void handleMessage(cMessage *msg) override;
};

}; //namespace

#endif
