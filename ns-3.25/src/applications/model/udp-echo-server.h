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

#ifndef UDP_ECHO_SERVER_H
#define UDP_ECHO_SERVER_H

#include <map>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "../src/applications/model/Raptor_Codes/src/libraptor/Array_Data_Types.h"

namespace ns3 {


class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class UdpEchoServer : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  UdpEchoServer ();
  virtual ~UdpEchoServer ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket); 
  
  void sendAck();

  int RoundUp(int numToRound, int multiple);

  void Raptor_Decoder(std::map<uint16_t, std::map<uint32_t, std::vector<uint8_t> > > &rec, uint16_t blockNum);

  void checkDecoded(Array_Data_Symbol source, uint32_t z);

  void sendBack(Ptr<Socket> soc, Address from, Ptr<Packet> packet);

  std::vector<uint8_t> packetSymbols;

  //Array_Data_Symbol encodingSymbols(uint32_t k, uint16_t t);


  std::vector<uint8_t>::iterator iter;
  std::vector<uint32_t>::iterator iter1;

   uint16_t m_received;
   uint8_t 	m_Reserved;

   std::map<uint16_t, 
   			std::map<uint32_t, std::vector<uint8_t> > > m_blocks;

   std::map<uint16_t, 
   			std::map<uint32_t, std::vector<uint8_t> > >::iterator main_itr;

   	std::map<uint16_t, 
   			std::map<uint32_t, std::vector<uint8_t> > >::iterator itr;

   //std::multimap<std::pair<uint16_t, uint32_t>, std::vector<uint8_t> > m_blocks;
   //std::multimap<std::pair<uint16_t, uint32_t>, std::vector<uint8_t> >::iterator main_itr;

    
   std::map<uint32_t, std::vector<uint8_t> >::iterator inner_itr;
   
   uint16_t m_currentBlockCount;
   uint16_t m_nextBlockCount;
   int m_Gcount;

   uint32_t m_ESI;
   uint32_t m_reqBytesPerBlock;
   uint32_t m_receivedBytesPerBlock;

   uint32_t m_totalBytesReceived;
   uint16_t m_lastSeq;       //!< maximum received sequence number
   uint16_t m_currentSeq;    //!< sequence number send - current
   uint16_t m_rwnd;          //!< receiver window
   uint16_t m_flag;          //!< done msg byte
   uint8_t* m_data;          //!< ack data to send
   //bool visited;
   uint64_t m_totalReqBytes;

   uint64_t m_transferLength;
   uint8_t m_Al;
   uint16_t m_symbolLen;
   uint32_t m_numberSrcBlck;
   uint8_t m_numberSubBlck;

   double m_G;
   double m_Kt;

   uint32_t m_KL;
   uint32_t m_KS;
   uint32_t m_ZL;
   uint32_t m_ZS;

   uint32_t m_TL;
   uint32_t m_TS;
   uint32_t m_NL;
   uint32_t m_NS;
   uint32_t m_N;

   uint16_t m_overhead;
 
   uint16_t m_port;          //!< port on which we listen for incoming packets.
  
  
  Ptr<Socket> m_socket;     //!< IPv4 Socket
  Ptr<Socket> m_socket6;    //!< IPv6 Socket
  Address m_local;          //!< local multicast address
  
   // Callbacks for tracing the packet Tx events
   TracedCallback<Ptr< const Packet> > m_rxTrace;
};

} // namespace ns3

#endif /* UDP_ECHO_SERVER_H */

