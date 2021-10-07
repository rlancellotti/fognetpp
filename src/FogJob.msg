


namespace fog;

//
// FogJob message definition
//
packet FogJob
{
    //@customize(true);
    simtime_t startTime; 		// time the message is sent
    simtime_t queuingTime; 		// total time spent standing in queues
    simtime_t serviceTime;  	// total time spent in servers
    simtime_t delayTime;    	// total time spent in delay modules
    simtime_t balancerTime;    	// total time spent in Load balancing modules
    simtime_t probeTime;		// time passed waiting for probe
    simtime_t slaDeadline;		// deadline for SLA (-1 for no deadline)
	simtime_t suggestedTime;	// suggested service time. 
	simtime_t suggestedDelay;	// suggested delay time. 
    
    int queueCount;             // the number of queue modules visited by the job
    int delayCount;             // the number of delay modules visited by the job
    int balancerCount;			// the number of load balancing modules visited by the job
    int appId;					// ID of the considered fog App
    //int id;					// id of the msg
    double previousLoad;		// load of the node that sent the message
    double expectedLoad;		// expectation of remote load on the node the sent the message
}

