#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("One_Tcp");

uint32_t m_bytesTotal = 0;

static void
ReceivedPacket(Ptr<const Packet> p, const Address & addr){

m_bytesTotal += p->GetSize (); 
NS_LOG_UNCOND( "Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);

}



int
main (int argc, char *argv[]){

  uint32_t maxBytes = 5120000;
  uint32_t sendSize = 1000;
   
     
   
  Time::SetResolution (Time::NS);
  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("TcpSocketBase", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1000));

  NodeContainer c;
  c.Create(6);
  
  NodeContainer n0n1 = NodeContainer (c.Get(0), c.Get(1));
  NodeContainer n1n2 = NodeContainer (c.Get(1), c.Get(2));
  NodeContainer n2n3 = NodeContainer (c.Get(2), c.Get(3));
  NodeContainer n3n4 = NodeContainer (c.Get(3), c.Get(4));
  NodeContainer n4n5 = NodeContainer (c.Get(4), c.Get(5));
  
  //Install Iternet Stack
  InternetStackHelper internet;
  internet.Install(c);
  
  
  NS_LOG_INFO("Create Channels");
  PointToPointHelper pp;
  pp.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pp.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(1000));

  
  NetDeviceContainer d0d1 = pp.Install(n0n1);
  NetDeviceContainer d1d2 = pp.Install(n1n2);
  NetDeviceContainer d2d3 = pp.Install(n2n3);
  NetDeviceContainer d3d4 = pp.Install(n3n4);
  NetDeviceContainer d4d5 = pp.Install(n4n5);
  
  //Assign IP Address 
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);
  
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);
  
  ipv4.SetBase("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);
  
  ipv4.SetBase("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);
  
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
  
  NS_LOG_INFO("Enable global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
  
  
  NS_LOG_INFO("Create Applications");
  
  //create bulk send on 0
  uint16_t sinkPort = 90;
  BulkSendHelper bulkSend ("ns3::TcpSocketFactory", InetSocketAddress (i4i5.GetAddress (1), sinkPort));
   bulkSend.SetAttribute("MaxBytes", UintegerValue (maxBytes));
    bulkSend.SetAttribute("SendSize", UintegerValue (sendSize));
   ApplicationContainer sourceApps = bulkSend.Install(c.Get(0));
   sourceApps.Start(Seconds(0.0));
 
  //create packet sink install on 2 
   PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
   ApplicationContainer sinkApps = packetSinkHelper.Install(c.Get (5));
   sinkApps.Start(Seconds(0.0));
   
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&ReceivedPacket));
  //Simulator::Schedule(Seconds(100000), &Throughput);  

  AsciiTraceHelper ascii;
  pp.EnableAsciiAll (ascii.CreateFileStream ("tcp-1pair.tr"));
  pp.EnablePcapAll ("tcp-1pair", true);
    
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
  
}
