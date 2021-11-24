namespace fog;

//
// Multi-Service Job message definition
//
packet MultiJob
{
    simtime_t startTime;        // time the message is sent
    simtime_t slaDeadline;      // deadline for SLA (-1 for no deadline)
	simtime_t suggestedTime[];  // suggested service time. Array of values
    int outputs[];				// IDs of the output routes traversed
    int chain;                  // ID of the service chain -- similar to AppID
    int serviceCount;           // the number of services traversed by the job
    int queueCount;             // the number of queue modules visited by the job
    int delayCount;             // the number of delay modules visited by the job
    simtime_t queuingTime;      // total time spent standing in queues
    simtime_t serviceTime;      // total time spent in servers
    simtime_t delayTime;        // total time spent in delay modules
}

