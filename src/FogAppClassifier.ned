//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package fog;

//
// Sends the messages to different outputs depending on messages app ID
// If no output found for a message, it is dropped
// Messages with app = i will be sent to out[i]
//
simple FogAppClassifier
{
    parameters:
        @display("i=block/classifier");
    gates:
        input in[];
        output out[];
        output rest;
}
