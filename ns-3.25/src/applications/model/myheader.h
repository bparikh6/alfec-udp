
namespace ns3{

typedef struct {
     uint16_t currentSeq;  	//Sent sequence number
     uint16_t SBN;		     //Source block number of each encoded packet
     uint32_t ESI;		     //Encoding Symbol id for each packet/encoded symbol
} ECSendHeader;



typedef struct {
     uint16_t lastSeq;		//Highest received sequence number
     uint16_t rwnd;			//Receiver's window / advertised window
     uint32_t nextBlock;      //To send next block
     uint16_t flag;
} ECRecvHeader;




typedef struct {
	uint64_t F;			//Trandfer Length (total data length)
	uint8_t  Al;			//Symbol Alignment parameter
	uint16_t T;			//Symbol lenght
	uint32_t Z;			//Number of source blocks
     uint8_t  N;			//Number of sub blocks
} CDPHeader;


}

// OTI

// F,T,Al,Z,N
