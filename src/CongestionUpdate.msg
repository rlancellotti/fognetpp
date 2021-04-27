
namespace fog;

//
// CongestionUpdate message definition for M/G/n simulations
//
message CongestionUpdate
{
    //@customize(true);
    bool congestion;			// Congestion status: true or false
    double multiplier;			// multiplier for service/delay time
}


