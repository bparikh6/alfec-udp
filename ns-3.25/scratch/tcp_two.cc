#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Two_Tcp");

uint32_t m_bytesTotal = 0;

static void
ReceivedPacket(Ptr<const Packet> p, const Address & addr){

m_bytesTotal += p->GetSize (); 
NS_LOG_UNCOND( "Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);

}

void
Throughput(){

double mbps = (m_bytesTotal)/1000;
double time = Simulator::Now ().GetSeconds ();
std::cout << "time " << time << " Throughput: " << mbps << std::endl;

}


int
main (int argc, char *argv[]){

  uint32_t maxBytes = 1024000;
  uint32_t sendSize = 1000;
   
   
   
   
  Time::SetResolution (Time::NS);
  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  
 Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
 Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1000));

  NodeContainer c;
  c.Create(8);
  
  NodeContainer n0n4 = NodeContainer (c.Get(0), c.Get(4));
  NodeContainer n1n4 = NodeContainer (c.Get(1), c.Get(4));
  NodeContainer n2n7 = NodeContainer (c.Get(2), c.Get(7));
  NodeContainer n3n7 = NodeContainer (c.Get(3), c.Get(7));
  NodeContainer n4n5 = NodeContainer (c.Get(4), c.Get(5));
  NodeContainer n5n6 = NodeContainer (c.Get(5), c.Get(6));
  NodeContainer n6n7 = NodeContainer (c.Get(6), c.Get(7));
  
  //Install Iternet Stack
  InternetStackHelper internet;
  internet.Install(c);
  
  
  NS_LOG_INFO("Create Channels");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p.SetQueue("ns3::DropTailQueue", "MaxPackets" , UintegerValue(1000));
  
  NetDeviceContainer d0d4 = p2p.Install(n0n4);
  NetDeviceContainer d1d4 = p2p.Install(n1n4);
  NetDeviceContainer d2d7 = p2p.Install(n2n7);
  NetDeviceContainer d3d7 = p2p.Install(n3n7);
  NetDeviceContainer d4d5 = p2p.Install(n4n5);
  NetDeviceContainer d5d6 = p2p.Install(n5n6);
  NetDeviceContainer d6d7 = p2p.Install(n6n7);
  
  //Assign IP Address 
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i4 = ipv4.Assign(d0d4);
  
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i4 = ipv4.Assign(d1d4);
  
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i7 = ipv4.Assign(d2d7);
  
  ipv4.SetBase("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i7 = ipv4.Assign(d3d7);
  
  ipv4.SetBase("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
  
  ipv4.SetBase("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);
  
  ipv4.SetBase("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i6i7 = ipv4.Assign(d6d7);
  
  
  
  NS_LOG_INFO("Enable global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
  
  
    
  NS_LOG_INFO("Create Applications");
  
  //create bulk send on 0
   uint16_t sinkPort = 90;
   BulkSendHelper bulkSend ("ns3::TcpSocketFactory", InetSocketAddress (i2i7.GetAddress (0), sinkPort));
   bulkSend.SetAttribute("MaxBytes", UintegerValue (maxBytes));
   bulkSend.SetAttribute("SendSize", UintegerValue (1000));
   ApplicationContainer sourceApps = bulkSend.Install(c.Get(0));
   sourceApps.Start(Seconds(0.0));
 
  //create packet sink install on 2 
   PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
   ApplicationContainer sinkApps = packetSinkHelper.Install(c.Get (2));
   sinkApps.Start(Seconds(0.0));
     
   
  //create bulk send on 1
   uint16_t sinkPort1 = 8080;
   BulkSendHelper bulkSend1 ("ns3::TcpSocketFactory", InetSocketAddress (i3i7.GetAddress (0), sinkPort1));
   bulkSend1.SetAttribute("MaxBytes", UintegerValue (maxBytes));
   bulkSend1.SetAttribute("SendSize", UintegerValue (sendSize));
   ApplicationContainer sourceApps1 = bulkSend1.Install(c.Get(1));
   sourceApps1.Start(Seconds(0.0));
 
  //create packet sink install on 3 
   PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort1));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install(c.Get (3));
   sinkApps1.Start(Seconds(0.0));
   
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&ReceivedPacket));
  Simulator::Schedule(Seconds(1000000), &Throughput);  

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("tcp_2pair.tr"));
 // p2p.EnablePcapAll ("tcp_2pair", true);
      
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
  
}
