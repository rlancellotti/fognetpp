//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package fog;

//
// A queue with a built-in server
//
simple CloudCongestionControl
{
    parameters:
        @display("i=block/cogwheel");
        volatile double toCongestionStatusInterval @unit(s) = default(exponential(10s));
        volatile double fromCongestionStatusInterval @unit(s) = default(exponential(10s));
        double congestionMultiplier = default(1.3);

    gates:
        output congestionControl[];
}
