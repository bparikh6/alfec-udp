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
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/udp-header.h"
#include "ns3/sequence-number.h"
#include "myheader.h"

#include "udp-echo-server.h"

#include "Raptor_Codes/src/libraptor/Partition.h"
#include "Raptor_Codes/src/libraptor/RandNum_Generator.h"
#include "Raptor_Codes/src/libraptor/Degree_Generator.h"
#include "Raptor_Codes/src/libraptor/rfc5053_config.h"
#include "Raptor_Codes/src/libraptor/Array_Data_Types.h"
#include "Raptor_Codes/src/libraptor/Inter_Symbol_Generator.h"
#include "Raptor_Codes/src/libraptor/R10_Decoder.h"
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

NS_LOG_COMPONENT_DEFINE ("UdpEchoServerApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpEchoServer);

TypeId
UdpEchoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpEchoServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpEchoServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&UdpEchoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Rx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpEchoServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

UdpEchoServer::UdpEchoServer ()
{
  NS_LOG_FUNCTION (this);
  m_data = (uint8_t*) malloc(sizeof(ECRecvHeader));
  m_totalBytesReceived = 0;
  m_lastSeq = 0;
  m_currentSeq = 0;
  m_rwnd = 65535;
  m_received = 0;

  m_transferLength = 0;
  m_Al = 0;
  m_symbolLen = 0;
  m_numberSrcBlck = 0;
  m_numberSubBlck = 0;

  m_totalReqBytes = 0;
  m_receivedBytesPerBlock = 0;

  m_Kt = 0;
  m_ZL = 0;
  m_ZS = 0;
  m_KL = 0;
  m_KS = 0;
  m_TL = 0;
  m_TS = 0;
  m_NL = 0;
  m_NS = 0;
  m_G = 0;

  m_nextBlockCount = 0;

}

UdpEchoServer::~UdpEchoServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
UdpEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpEchoServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      m_socket6->Bind (local6);
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpEchoServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpEchoServer::HandleRead, this));

}

void 
UdpEchoServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
UdpEchoServer::HandleRead (Ptr<Socket> socket)
{
  	UdpHeader header;
  	NS_LOG_FUNCTION (this << socket);

  	Ptr<Packet> packet;
  	Address from;

  
    while (packet = socket->RecvFrom (from))
    {
    	//receive data packets/encoding symbols
    	if(m_received > 0)
    	{

        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData (buffer, packet->GetSize ());
  
        ECSendHeader *hdr = (ECSendHeader *)buffer;
        uint8_t *payload = buffer + sizeof( ECSendHeader);    
    
        m_currentSeq  		  = hdr->currentSeq;
        m_currentBlockCount = hdr->SBN;
        m_ESI		  		      = hdr->ESI;

        
        NS_LOG_INFO("\nBlock Number is at Node(" << GetNode()->GetId() << ") " << m_currentBlockCount << " ESI is " << m_ESI);  

        
        m_reqBytesPerBlock = (m_currentBlockCount < m_ZL) ? 
                                  RoundUp(m_KL*(0.05 + 1)*m_symbolLen, 1000) : 
                                              RoundUp(m_KS*(0.05 + 1)*m_symbolLen, 1000);

        //NS_LOG_INFO("Total bytes and Required symbols per block " << m_totalReqBytes << "\t" << m_reqBytesPerBlock);

        if(m_totalBytesReceived < m_totalReqBytes) 
        {
            

            if( (m_receivedBytesPerBlock < m_reqBytesPerBlock) && (m_currentBlockCount == m_nextBlockCount) ){


                  m_totalBytesReceived =  m_totalBytesReceived + packet->GetSize () - sizeof(ECSendHeader) ;

                  for(unsigned i = 0; i < (packet->GetSize() - sizeof(ECSendHeader)); ++i){
                          packetSymbols.push_back(payload[i]);
                  }
                  // receive and populate the map with current block
                  for(iter = packetSymbols.begin(); iter != packetSymbols.end(); ++iter){
                        m_blocks[m_currentBlockCount][m_ESI].push_back(*iter);  
                  }
            
                  m_receivedBytesPerBlock = m_receivedBytesPerBlock + m_blocks[m_currentBlockCount][m_ESI].size();
              
                  NS_LOG_INFO("Previous block " << m_nextBlockCount);

                  NS_LOG_INFO("Size of map is " << m_blocks.size() << "\t"
                              << " Size of inner map " << m_blocks[m_currentBlockCount].size() << "\t"
                              << " Size of vector " << m_blocks[m_currentBlockCount][m_ESI].size() 
                              << " Required Bytes " << m_totalReqBytes);

                  packetSymbols.clear();

                  m_rxTrace(packet);

                  if (InetSocketAddress::IsMatchingType (from) ){
                              NS_LOG_INFO ("At time " 
                                          << Simulator::Now ().GetSeconds() 
                                          << " Node(" << GetNode()->GetId() 
                                          << ") received "  << packet->GetSize () 
                                          << " bytes from " 
                                          << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                                          << " port " 
                                          << InetSocketAddress::ConvertFrom (from).GetPort ());
                  }
        
                  else if (Inet6SocketAddress::IsMatchingType (from)){
                              NS_LOG_INFO ("At time " 
                                          << Simulator::Now ().GetSeconds()
                                          << " Node(" << GetNode()->GetId() 
                                          << ") received " << packet->GetSize () 
                                          << " bytes from " 
                                          << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () 
                                          << " port " 
                                          << Inet6SocketAddress::ConvertFrom (from).GetPort ());
                  }
       
                  packet->RemoveAllPacketTags ();
                  packet->RemoveAllByteTags ();

                  
            } 
            else if( (m_receivedBytesPerBlock == m_reqBytesPerBlock) && (m_currentBlockCount == m_nextBlockCount) ){
                // decode each block and check
                Raptor_Decoder(m_blocks, m_currentBlockCount);
                m_receivedBytesPerBlock = 0;
                m_nextBlockCount += 1;
                m_blocks.clear();

                packet->RemoveAllPacketTags ();
                packet->RemoveAllByteTags ();
                //Send an Acknowledgemnet
    
                  ECRecvHeader *hdr_ = (ECRecvHeader*) m_data;
                  hdr_->lastSeq      = std::max(m_currentSeq, m_lastSeq);
                  hdr_->rwnd         = m_rwnd;
                  hdr_->nextBlock    = m_nextBlockCount;
                  hdr_->flag         = 0;
    
                  Ptr<Packet> p = Create<Packet>(m_data, sizeof(ECRecvHeader));
      
                  m_lastSeq = hdr_->lastSeq;        
                  NS_LOG_INFO("Last Seq is --- " << m_lastSeq);
     
                  NS_LOG_INFO ("Echoing packet with delayyy");
                  
                  socket->SendTo (p, 0, from);

                  void (UdpEchoServer::*fp)(Ptr<Socket>, Address, Ptr<Packet>) = &UdpEchoServer::sendBack;

                  if(m_nextBlockCount >= m_ZL){
                    Simulator::Schedule(Seconds(2000), fp, this, socket, from, p);
                  }
                  else{
                    NS_LOG_INFO("Here");
                    Simulator::Schedule(Seconds(0.008), fp, this, socket, from, p);

                  }
                  
                  Simulator::Schedule(Seconds(0.008), fp, this, socket, from, p);
                  Simulator::Schedule(Seconds(0.008), fp, this, socket, from, p);
                  Simulator::Schedule(Seconds(0.008), fp, this, socket, from, p);


                  if (InetSocketAddress::IsMatchingType (from)){
                            NS_LOG_INFO ("At time " 
                                        << Simulator::Now ().GetSeconds() 
                                        << "s Node(" << GetNode()->GetId() 
                                        << ") " <<" server sent " 
                                        << p->GetSize () << " bytes to " 
                                        << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                                        << " port " 
                                        << InetSocketAddress::ConvertFrom (from).GetPort ());
                  }
                  else if (Inet6SocketAddress::IsMatchingType (from)){
                            NS_LOG_INFO ("At time " 
                                         << Simulator::Now ().GetSeconds()
                                         << "s Node(" << GetNode()->GetId()
                                         << ") " <<"s server sent " 
                                         << p->GetSize () << " bytes to " 
                                         << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () 
                                         << " port " 
                                         << Inet6SocketAddress::ConvertFrom (from).GetPort ());
                  }
                
            }
            
        }//end if(m_totalReceived <= m_totalReqBytes)

        else 
        {
              
              //Send Stop message/ack
              ECRecvHeader *hdr_ = (ECRecvHeader*) m_data;
              hdr_->lastSeq      = std::max(m_currentSeq, m_lastSeq);
              hdr_->rwnd         = m_rwnd;
              hdr_->nextBlock    = m_nextBlockCount;
              hdr_->flag         = 1;
    
              Ptr<Packet> p = Create<Packet>(m_data, sizeof(ECRecvHeader));
     
              NS_LOG_LOGIC ("Echoing packet");
              socket->SendTo (p, 0, from);
              void (UdpEchoServer::*fp)(Ptr<Socket>, Address, Ptr<Packet>) = &UdpEchoServer::sendBack;
                  
              Simulator::Schedule(Seconds(200), fp, this, socket, from, p);


                if (InetSocketAddress::IsMatchingType (from)){
                    NS_LOG_INFO ("At time " 
                           << Simulator::Now ().GetSeconds () 
                           << " s Node(" << GetNode()->GetId() 
                           << ") " <<" sent stop message to client ");
                }
                else if (Inet6SocketAddress::IsMatchingType (from)){
                    NS_LOG_INFO ("At time " 
                            << Simulator::Now ().GetSeconds () 
                            << " s Node(" << GetNode()->GetId() 
                            << ") " <<" sent stop message to client ");
                }
                UdpEchoServer::StopApplication();

        }// end else
    
      }//end if(m_received > 0)
        
      //receive CDP requirements
      else
      {

    		//receive CDP header
    		uint8_t *buff = new uint8_t[packet->GetSize()];
    		packet->CopyData (buff, packet->GetSize ());

    		CDPHeader *cdp    = (CDPHeader *) buff;
    		m_transferLength 	= cdp->F;
    		m_Al 				      = cdp->Al;
    		m_symbolLen			  = cdp->T;
    		m_numberSrcBlck		= cdp->Z;
    		m_numberSubBlck		= cdp->N;
    		
    		++m_received;
    		m_rxTrace(packet);

  			m_Kt = std::ceil(m_transferLength/m_symbolLen);
  			m_G = std::min(ceil((double)(1000 * 1024) / (double)m_transferLength), (double)(1000 / m_Al));
  			m_G = std::min(m_G, (double)10);
  		
  			NS_LOG_INFO("\n At Node ("<< GetNode()->GetId() << ") " << "Got F, Al ! " << m_transferLength << "\t" << (uint16_t)m_Al);
    		NS_LOG_INFO("T, Z, N, Kt, G " << m_symbolLen <<"\t"
    								<< m_numberSrcBlck << "\t"
    								<< (int)m_numberSubBlck << "\t"
    								<< m_Kt << "\t"
    								<< m_G	);

  			
  			m_KL = 1024;
  			m_KS = 1024;
  			m_ZL = m_numberSrcBlck; 
            m_ZS = 0;
  			
  			NS_LOG_INFO("KL, KS, ZL, ZS is " << m_KL << "\t"
	    									<< m_KS << "\t"
	    									<< m_ZL << "\t"
	    									<< m_ZS << "\t");

        m_totalReqBytes = RoundUp(m_KL*m_ZL*0.25*m_symbolLen, m_symbolLen) 
                                            + RoundUp(m_KS*m_ZS*0.25*m_symbolLen, m_symbolLen) 
                                                                                  + m_transferLength;
        //Send Ack
    		ECRecvHeader *hdr_ = (ECRecvHeader*) m_data;
    	  hdr_->lastSeq      = std::max(m_currentSeq, m_lastSeq);
    	  hdr_->rwnd         = m_rwnd;
    	  hdr_->nextBlock    = m_nextBlockCount; 
        hdr_->flag         = 0;
    
   		  Ptr<Packet> p = Create<Packet>(m_data, sizeof(ECRecvHeader));
   		
   		  m_lastSeq = hdr_->lastSeq;        
        
     
     	  //NS_LOG_LOGIC ("Echoing packet");
      	socket->SendTo (p, 0, from);

        void (UdpEchoServer::*fp)(Ptr<Socket>, Address, Ptr<Packet>) = &UdpEchoServer::sendBack;
                  
        Simulator::Schedule(Seconds(0.005), fp, this, socket, from, p);

        Simulator::Schedule(Seconds(0.005), fp, this, socket, from, p);
      	        

    	}//end else	
   	
    }//end while  

}//end UdpEchoServer::HandleRead

void
UdpEchoServer::Raptor_Decoder(std::map<uint16_t, std::map<uint32_t, std::vector<uint8_t> > > &recvMap, uint16_t m_blockNum){

    uint32_t K_ = (m_blockNum < m_ZL) ? m_KL : m_KS;
    Array_Data_Symbol encodingSymbols(K_, m_symbolLen);
    
    std::vector<uint32_t> ESI;
    uint32_t c = 0;

    itr = recvMap.find(m_blockNum); 

    //NS_LOG_INFO(" Encoding Symbol size is " << encodingSymbols.symbol.size());


    if(itr != recvMap.end()){
      
        for(inner_itr = itr->second.begin(); inner_itr != itr->second.end(); ++inner_itr){
            
            //NS_LOG_INFO("ESI and ESI + G is " << inner_itr->first << "\t" << inner_itr->first + m_G);
            
            for(int i = inner_itr->first; i < inner_itr->first + m_G; ++i ){
              ESI.push_back(i);
              encodingSymbols.ESIs.push_back(i);
              }

              //NS_LOG_INFO("After assigning ESIs Encoding Symbol size is " << encodingSymbols.ESIs.size() 
                                 // << "\t" <<encodingSymbols.symbol.size());

              iter = inner_itr->second.begin();
              for(int i = inner_itr->first; i < inner_itr->first + m_G; ++i){
                ++c;
                for(int j = 0; j < (int) m_symbolLen; ++j){
                  //NS_LOG_INFO(" I and J " << i << "\t" << j);
                  encodingSymbols.symbol[c-1].s[j] = *iter;
                  //std::cout << (int) *iter;
                  ++iter;
                  if(iter == inner_itr->second.end())
                    break;
                }
              }
        }
       // NS_LOG_INFO("Encoding Symbol size is " << encodingSymbols.K << "\t" << encodingSymbols.symbol.size());

    }
    
    //Perform Decoding
    R10_Decoder decoder(K_, m_symbolLen);

    Array_Data_Symbol source = decoder.Get_Source_Symbol(encodingSymbols, encodingSymbols.symbol.size());

    checkDecoded(source, m_blockNum);

    //recvMap.clear();
	
}

void
UdpEchoServer::checkDecoded(Array_Data_Symbol source, uint32_t m_blockNum){

   uint32_t K_ = (m_blockNum < m_ZL) ? m_KL : m_KS;

  Array_Data_Symbol testing_symbol(K_, m_symbolLen);

    if(m_blockNum < m_ZL)
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

    for (int i = 0; i < (int)source.symbol.size(); ++i)
    {
      for (int j = 0; j < (int) m_symbolLen; ++j){
        if ( ( (source.symbol[i].s[j]) ^ (testing_symbol.symbol[i].s[j]) ) != 0)
        {

             std::cout << " decode fail! " << std::endl;
             break;

        }
      }

    }

   std::cout << "At time "<< Simulator::Now().GetSeconds() <<" Decoding Successful " << std::endl;

}


int
UdpEchoServer::RoundUp(int numToRound, int multiple){
  
    if (multiple == 0)
        return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

  
  return numToRound + multiple - remainder;

}

void
UdpEchoServer::sendBack(Ptr<Socket> soc, Address from, Ptr<Packet> packet)
{
         soc->SendTo (packet, 0, from);

}


} // Namespace ns3

