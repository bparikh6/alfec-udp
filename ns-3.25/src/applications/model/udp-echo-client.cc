/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "udp-echo-client.h"
#include "myheader.h"

#include "Raptor_Codes/src/libraptor/Partition.h"
#include "Raptor_Codes/src/libraptor/RandNum_Generator.h"
#include "Raptor_Codes/src/libraptor/Degree_Generator.h"
#include "Raptor_Codes/src/libraptor/rfc5053_config.h"
#include "Raptor_Codes/src/libraptor/Array_Data_Types.h"
#include "Raptor_Codes/src/libraptor/Inter_Symbol_Generator.h"
//#include "Raptor_Codes/src/libraptor/R10_Decoder.h"
#include "Raptor_Codes/src/libraptor/LT_Encoding.h"


#include "Raptor_Codes/src/libraptor/Utility.h"


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include "Raptor_Codes/src/libraptor/storage_adaptors.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpEchoClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpEchoClient);

TypeId
UdpEchoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpEchoClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpEchoClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpEchoClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpEchoClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpEchoClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpEchoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&UdpEchoClient::SetDataSize,
                                         &UdpEchoClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AppendOverhead", "Number of extra overhead required for encoding (In percentage)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpEchoClient::m_reqOverhead),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("TransferLength", 
    			   "Total size of actual data to transfer",
    			   UintegerValue(1024000),
    			   MakeUintegerAccessor (&UdpEchoClient::m_transferLength),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("NumOfBlocks", 
    			   "Number of blocks the data has to be partitioned to",
    			   UintegerValue(1),
    			   MakeUintegerAccessor (&UdpEchoClient::m_totalNumBlocks),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpEchoClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

UdpEchoClient::UdpEchoClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_dataSize = 1000;
  m_data = (uint8_t*) malloc(m_dataSize + sizeof(ECSendHeader));
  m_parameters = (uint8_t*) malloc(sizeof(CDPHeader));
  m_lastSeq = 0;
  m_lastAck = 0;
  m_rwnd = 1;
  m_currentSeq = 0;
  m_sentBytes = 0;
  m_overhead = 0;
  m_blocksCount = 0;
  m_G = 0;

  m_Al = 0;
  m_symbolLen = 0;
  m_numberSrcBlck = 0;
  m_numberSubBlck = 0;

  m_ZL = 0;
  m_ZS = 0;
  m_KL = 0;
  m_KS = 0;
  m_K  = 0;
  m_received = 0;

}

UdpEchoClient::~UdpEchoClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  free(m_data);
  m_data = 0;
  m_dataSize = 0;
}

void 
UdpEchoClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpEchoClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void 
UdpEchoClient::SetRemote (Ipv6Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
UdpEchoClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpEchoClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      
      //create socket
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind6();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
    }
 
  //Calculate Kt, G, T, Z, N
  struct CDP_Parameters_Input param;
  
  param.F 		= m_transferLength;
  param.W 		= 64;
  param.P 		= 1000;
  param.Al 		= 4;
  param.Kmax 	= 8192;
  param.Kmin	= 1024;
  param.Gmax 	= 10; 

  CDP_Transport_Parameters calculate_param;
  CDP_Parameters_Output calculatedParameters = calculate_param.CDP_Parameters_caculate(param);

  /*NS_LOG_INFO("Sender sent values T, Z, N, Kt, G " 
					<< calculatedParameters.T << "\t"
	   				<< calculatedParameters.Z << "\t"
	   				<< (uint16_t)calculatedParameters.N << "\t"  
	   				<< calculate_param.Kt << "\t"
	   				<< calculate_param.G
	   				);*/

  m_Al 					= param.Al;
  m_symbolLen			= calculatedParameters.T;
  m_numberSrcBlck		= m_totalNumBlocks;
  m_numberSubBlck		= calculatedParameters.N;

  m_Kt = calculate_param.Kt;
  m_G  = calculate_param.G;

  //Calculate the values KL, KS, ZL, ZS, TL,TS, NL, NS
  m_KL = 1024;
  m_KS = 1024;
  m_ZL = m_totalNumBlocks; 
  m_ZS = 0;
  /*NS_LOG_INFO("KL, KS, ZL, ZS is " << m_KL << "\t"
	    	<< m_KS << "\t"
	    	<< m_ZL << "\t"
	    	<< m_ZS << "\t");*/
  NS_LOG_INFO("Total Number of Blocks " << m_ZL + m_ZS );

  m_totalBytesToSend = m_transferLength + m_ZL*m_KL*m_symbolLen*(double)(m_reqOverhead/100); 

  m_socket->SetRecvCallback (MakeCallback (&UdpEchoClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
  
}

void 
UdpEchoClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void 
UdpEchoClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
UdpEchoClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpEchoClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  /*uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_chunk;
      m_chunk = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_chunk, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;*/
}

void 
UdpEchoClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
 
 /* if (dataSize != m_dataSize)
    {
      delete [] m_chunk;
      m_chunk = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_chunk, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;*/
}

void 
UdpEchoClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  /*if (dataSize != m_dataSize)
    {
      delete [] m_chunk;
      m_chunk = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_chunk, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_chunk[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_chunk[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;*/
}



void 
UdpEchoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  NS_ASSERT(m_sendEvent.IsExpired());
  m_sendEvent = Simulator::Schedule (dt, &UdpEchoClient::Send, this);
}

void 
UdpEchoClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  unsigned count = 0;
    
    //sent data bytes
 
  if(m_sent > 0)
  {

		/*NS_LOG_INFO("\nSize of list ..................................... " << m_block.size() << "\t" 
			            << " Current block sending " << m_blocksCount << "\t" 
			            << " To send after receiving ACK " << m_blockToSend);*/


		if(m_blocksCount != m_blockToSend){
    	   	m_blocksCount = m_blockToSend;
		  	m_block.clear();
			m_K = 0;
			encodedSymbols = 0;
    	}
    	        
    	//NS_LOG_INFO("Block Size at Node("<< GetNode()->GetId()  <<") and ACK is " << m_blocksCount << "\t" << m_blockToSend);
    	        
    	if(m_block.size() == 0){
    					
    	    if(m_blocksCount < m_ZL + m_ZS){
    	        Array_Data_Symbol sourceSymbols = divideIntoBlocks(m_blocksCount);

    	        NS_LOG_INFO("Encoding start time " << Simulator::Now().GetMilliSeconds() << "ms for block number " << m_blocksCount);
    			encodedSymbols = Raptor_Encoding(sourceSymbols);
    			NS_LOG_INFO("Encoding end time " << Simulator::Now().GetMilliSeconds() << "ms for block number " << m_blocksCount);

    			m_block.push_back(encodedSymbols);
    	    }
    		
    	}

		if(m_K < encodedSymbols.symbol.size()){
			//as encoded symbols size has been changed by overhead, we need not distinguish KL and KS now
			packetSymbols = dataToSendInEachPacket(encodedSymbols, m_K, m_G);
			//if m_G is more than 1, m_K will increase by G factor
			m_K += m_G;
		}
	
		//NS_LOG_INFO("m_K and encodedSymbols size " << m_K << "\t" << encodedSymbols.symbol.size());

		
		//Send State
		if((m_currentSeq - m_lastAck <= m_rwnd) && packetSymbols.size()!=0)
     	{
     			    
		     
			//copy packetSymbols (encoded symbols to send in each packet) into m_data buffer 
     		for(iter1 = packetSymbols.begin(); iter1 != packetSymbols.end(); ++iter1){
				m_data[count + sizeof(ECSendHeader)] = *iter1;
				count++;
			}

			//Attach the header for sequence number, blocknumber and ESI for each packet
            ECSendHeader *hdr    = (ECSendHeader *) m_data;
     		hdr->currentSeq      = ++m_lastSeq;
      		hdr->SBN 			 = m_blocksCount;
      		hdr->ESI 			 = m_K - m_G; 
      
      		/*NS_LOG_INFO("Size of data at Node(" 
      				        << GetNode()->GetId() 
      				        << ") is " << packetSymbols.size()
      				        << " Sequence numb " << hdr->currentSeq
      				        << " blockNum is " << hdr->SBN
      				        << " ESI is " << hdr->ESI);*/
    
      		p = Create<Packet>(m_data, packetSymbols.size() + sizeof(ECSendHeader));
      		
      		//clear packetSymbols for writing other contiguous encoded symbols 
      		packetSymbols.clear();
      
            m_currentSeq = hdr->currentSeq;
         
            m_sentBytes = m_sentBytes + p->GetSize() - sizeof(ECSendHeader);
            
         
  			// call to the trace sinks before the packet is actually sent,
  			// so that tags added to the packet can be sent as well
  			m_txTrace (p);  			
  			m_socket->Send (p);

  			++m_sent;

  			

  				if (Ipv4Address::IsMatchingType (m_peerAddress))
    			{
      				NS_LOG_INFO ("Node(" 
      				                << GetNode()->GetId() 
      				                << ") SENT packet of size " 
      				                << p->GetSize () 
      				                << " bytes to - " 
      				                << Ipv4Address::ConvertFrom (m_peerAddress) 
      				                << ":" << m_peerPort << "--------------" 
      				                << Simulator::Now ().GetSeconds());
    			}
  				else if (Ipv6Address::IsMatchingType (m_peerAddress))
    			{
      				NS_LOG_INFO ("Node(" 
      				                << GetNode()->GetId() 
      				                << ") SENT packet of size "
      				                << p->GetSize () 
      				                << " bytes to - " 
      				                << Ipv4Address::ConvertFrom (m_peerAddress) 
      				                << ":" << m_peerPort << "--------------" 
      				                << Simulator::Now ().GetSeconds() );
    			}

    			//NS_LOG_INFO("Sent Bytes " << m_sentBytes);

    		//wait state
        	if(m_currentSeq - m_lastAck > m_rwnd){
            	//Wait for another ack
        	}
        
            //send state
        	else if (m_currentSeq - m_lastAck <= m_rwnd){
       			ScheduleTransmit(m_interval);

       			//NS_LOG_INFO("Send event in " << m_sendEvent.IsExpired());
        	}	
        }

    	

  }
    
    //send CDP requirements
   else{
	    
	    NS_LOG_INFO("Node("<< GetNode()->GetId() << ") started at time " << Simulator::Now().GetSeconds());			
		
		//send CDP Header
		CDPHeader *cdp    = (CDPHeader *) m_parameters;
		cdp->F  = m_transferLength;
		cdp->Al = m_Al;
		cdp->T 	= m_symbolLen;
		cdp->Z	= m_numberSrcBlck;
		cdp->N 	= m_numberSubBlck;

	   	p = Create<Packet>(m_parameters, sizeof(CDPHeader));
	   	m_txTrace (p);

	   	m_socket->Send (p);

	   	void (UdpEchoClient::*fp)(Ptr<Socket>,Ptr<Packet>) = &UdpEchoClient::sendBack;
                  
        Simulator::Schedule(Seconds(0.005), fp, this, m_socket, p);
	   	
	   	++m_sent;
	    
   }
}


void
UdpEchoClient::HandleRead (Ptr<Socket> socket)
{
  
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;

        
    
  while ((packet = socket->RecvFrom (from)))
  {
        
        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData (buffer, packet->GetSize ());
    
        ECRecvHeader *recvhdr = (ECRecvHeader *)buffer;
    
        m_lastAck 		= recvhdr->lastSeq;
        m_rwnd    		= recvhdr->rwnd;
        m_blockToSend   = recvhdr->nextBlock;
        m_flag			= recvhdr->flag;
        
    	++m_received;
        /*NS_LOG_INFO("Receiver received Seq number " << m_lastAck << " Its Recvr Window is " << m_rwnd << " - block to send " 
        																	<< m_blockToSend << " Flag is " << m_flag);*/


        if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds() 
                        << " Node(" 
                        << GetNode()->GetId()  <<") received ------------------------ ACK "
                        << packet->GetSize () 
                        << " bytes from " 
                        << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                        << " port " 
                        << InetSocketAddress::ConvertFrom (from).GetPort ());
        }
        else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds()  
                        << " Node(" 
                        << GetNode()->GetId()  <<") received ------------------------ ACK " 
                        << packet->GetSize () 
                        << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () 
                        << " port " 
                        << Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
        
        //NS_LOG_INFO("Handle Read and block to send " << m_blockToSend << " Number of blocks " << m_ZL + m_ZS );
       
       //wait state
        if(m_currentSeq - m_lastAck > m_rwnd){

       		NS_LOG_INFO("In wait state");
       			
       		if ( (m_blockToSend >= m_ZL + m_ZS || m_flag == 1) && m_received == (int)m_ZL){
       			//UdpEchoClient::StopApplication();
       			//NS_LOG_INFO("Receiver of Node(" << GetNode()->GetId() <<") finished" );
       		}
       		else if (m_sendEvent.IsExpired()){
       			ScheduleTransmit(m_interval);
       	    }
        }
        //send state
        else if(m_currentSeq - m_lastAck <= m_rwnd){
       	
       		if ( (m_blockToSend >= m_ZL + m_ZS || m_flag == 1) && m_received == (int)m_ZL){
       			//NS_LOG_INFO("Receiver of Node(" << GetNode()->GetId() <<") finished" );
       		}
       		else if(m_sendEvent.IsExpired()){
       			//NS_LOG_INFO(" In HandleRead" );
       			ScheduleTransmit(m_interval);
       		}

       	}
             
  }

}

//divide object into blocks
Array_Data_Symbol
UdpEchoClient::divideIntoBlocks(uint16_t blockNum){

	uint32_t K_ = (blockNum < m_ZL) ? m_KL : m_KS;

	Array_Data_Symbol testing_symbol(K_, m_symbolLen);

		if(blockNum < (int)m_ZL)
		{
		 //NS_LOG_INFO("Value is - in if " << m_KL);

		 for(int i = 0; i < (int) m_KL; ++i){
		   for(int j = 0; j < (int) m_symbolLen; ++j ){
				if(j%3){
					testing_symbol.symbol[i].s[j] = 1;
				}
		   }
		 }
		
		}
		
		else
		{
	
		 //NS_LOG_INFO("Value is - in else " << m_KS);

		 for(int i = 0; i < (int) m_KS; ++i){
		   for(int j = 0; j < (int) m_symbolLen; ++j ){
				if(j%3){
					testing_symbol.symbol[i].s[j] = 1;
				}
		   }
		 }
		
		}

	return testing_symbol;

}


//encode each block independently
Array_Data_Symbol 
UdpEchoClient::Raptor_Encoding(Array_Data_Symbol sourceSymbols)
{

	NS_LOG_INFO("Required overhead " << (double)m_reqOverhead/100);
	
	Array_Data_Symbol enc(sourceSymbols.K, m_symbolLen);

	if(sourceSymbols.K == m_KL){
		
		double m_overheadCalc = (double) m_reqOverhead/100;
		m_overhead = sourceSymbols.K * m_overheadCalc;
		//NS_LOG_INFO("overhead is " << m_overhead);
		
		class LT_Encoding encoder(&sourceSymbols);
	
    	std::vector<uint32_t> ESI;
	
    	for (int i = 0; i < (int) sourceSymbols.K + m_overhead; ++i)
    	{
    		ESI.push_back(i);
    		enc.ESIs.push_back(i);
    	}
	
		//final encoded sequence
		enc.symbol = encoder.LTEnc_Generate(ESI);
		//NS_LOG_INFO("Encoding Done");

	}
	
	else{
		
		double m_overheadCalc = (double)m_reqOverhead/100;
		m_overhead = sourceSymbols.K * m_overheadCalc;

		class LT_Encoding encoder(&sourceSymbols);
	
    	std::vector<uint32_t> ESI;
	
    		for (int i = 0; i < (int) sourceSymbols.K + m_overhead; ++i)
    		{
    			ESI.push_back(i);
    			enc.ESIs.push_back(i);
    		}
	
	    //final encoded sequence
		enc.symbol = encoder.LTEnc_Generate(ESI);
		//NS_LOG_INFO("Encoding Done");
	}


	return enc;

	
}

//divide the independent block encoded symbols into the size of packet
std::vector<uint8_t> 
UdpEchoClient::dataToSendInEachPacket(Array_Data_Symbol encodedSymbols, uint32_t K_value, double G){
 		
 		uint32_t K = K_value + G;
		
		packetSymbol.clear();

		for(int i = K_value; i < (int) K; i++){
			if(i < (int)encodedSymbols.symbol.size()){
				for ( int j = 0; j < (int) m_symbolLen; j++){
				packetSymbol.push_back(encodedSymbols.symbol[i].s[j]); 
				}
			}
		}

	return packetSymbol;
}

void
UdpEchoClient::sendBack(Ptr<Socket> soc, Ptr<Packet> packet)
{
         soc->Send (packet);

}

} // Namespace ns3



//Implement m_currentSeq - m_lastAck < rwnd ---- send data
//m_higestAckreceived - m_lastAck >= rwnd ---- wait data, until m_higestAckreceived - m_lastAck < rwnd ---- send data
//when sending application done/receive done msg --- stop
