#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Dumbbell");

uint32_t m_bytesTotal = 0;

static void
ReceivedPacket(Ptr<const Packet> p){

m_bytesTotal += p->GetSize () - 8; 
NS_LOG_UNCOND( " Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);

}

void
Throughput(){

std::cout << "Received bytes " << m_bytesTotal << std::endl;
double KBps = (m_bytesTotal)/1000;
double time = Simulator::Now ().GetSeconds ();
std::cout << "time " << time << " Throughput: " << KBps << std::endl;
//m_bytesTotal = 0;
//Simulator::Schedule(Seconds(100), &Throughput);

}


int
main (int argc, char *argv[]){
  
  uint32_t sendSize = 1000;
  
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  NodeContainer c;
  c.Create(12);
  
  NodeContainer n0n4 = NodeContainer (c.Get(0), c.Get(4));
  NodeContainer n1n4 = NodeContainer (c.Get(1), c.Get(4));
  NodeContainer n8n4 = NodeContainer (c.Get(8), c.Get(4));
  NodeContainer n9n4 = NodeContainer (c.Get(9), c.Get(4));
  NodeContainer n2n7 = NodeContainer (c.Get(2), c.Get(7));
  NodeContainer n3n7 = NodeContainer (c.Get(3), c.Get(7));
  NodeContainer n10n7 = NodeContainer (c.Get(10), c.Get(7));
  NodeContainer n11n7 = NodeContainer (c.Get(11), c.Get(7));
  NodeContainer n4n5 = NodeContainer (c.Get(4), c.Get(5));
  NodeContainer n5n6 = NodeContainer (c.Get(5), c.Get(6));
  NodeContainer n6n7 = NodeContainer (c.Get(6), c.Get(7));
  
  //Install Iternet Stack
  InternetStackHelper internet;
  internet.Install(c);
  
  
  NS_LOG_INFO("Create Channels");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Kbps"));
  p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", StringValue("1000"));
  
  NetDeviceContainer d0d4 = p2p.Install(n0n4);
  NetDeviceContainer d1d4 = p2p.Install(n1n4);
  NetDeviceContainer d8d4 = p2p.Install(n8n4);
  NetDeviceContainer d9d4 = p2p.Install(n9n4);
  NetDeviceContainer d2d7 = p2p.Install(n2n7);
  NetDeviceContainer d3d7 = p2p.Install(n3n7);
  NetDeviceContainer d10d7 = p2p.Install(n10n7);
  NetDeviceContainer d11d7 = p2p.Install(n11n7);
  NetDeviceContainer d4d5 = p2p.Install(n4n5);
  NetDeviceContainer d5d6 = p2p.Install(n5n6);
  NetDeviceContainer d6d7 = p2p.Install(n6n7);
  
  //Assign IP Address 
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i4 = ipv4.Assign(d0d4);
  
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i4 = ipv4.Assign(d1d4);
  
  ipv4.SetBase("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer i8i4 = ipv4.Assign(d8d4);
  
  ipv4.SetBase("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i4 = ipv4.Assign(d9d4);
  
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
  
  ipv4.SetBase("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer i10i7 = ipv4.Assign(d10d7);
  
  ipv4.SetBase("10.1.11.0", "255.255.255.0");
  Ipv4InterfaceContainer i11i7 = ipv4.Assign(d11d7);
  
  
  NS_LOG_INFO("Enable global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  

 
  NS_LOG_INFO("Create Applications");
  
  //create applicaiton on 0
  uint16_t servPort = 8080;
  UdpEchoServerHelper echoServer(servPort);
  ApplicationContainer serverApps;
  serverApps = echoServer.Install (c.Get(2));
  serverApps.Start(Seconds(0.0));
  
  ApplicationContainer clientApps;
  
  UdpEchoClientHelper echoClient(i2i7.GetAddress(0), servPort);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.00432)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (sendSize));
  
  clientApps = echoClient.Install (c.Get (0));  
  clientApps.Start (Seconds (0.0));
  
  
  //create application on 1
  uint16_t servPort1 = 8000;
  UdpEchoServerHelper echoServer1(servPort1);
  ApplicationContainer serverApps1;
  serverApps1 = echoServer1.Install (c.Get(3));
  serverApps1.Start(Seconds(0.0));
  
  ApplicationContainer clientApps1;
  
  UdpEchoClientHelper echoClient1(i3i7.GetAddress(0), servPort1);
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (0.00432)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (sendSize));
  clientApps1 = echoClient1.Install (c.Get (1));  
  clientApps1.Start (Seconds (0.0));
  
 
  //create applicaiton on 8
  uint16_t servPort2 = 8880;
  UdpEchoServerHelper echoServer2(servPort2);
  ApplicationContainer serverApps2;
  serverApps2 = echoServer2.Install (c.Get(10));
  serverApps2.Start(Seconds(0.0));
  
  ApplicationContainer clientApps2;
  
  UdpEchoClientHelper echoClient2(i10i7.GetAddress(0), servPort2);
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (0.00432)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (sendSize));
  clientApps2 = echoClient2.Install (c.Get (8));  
  clientApps2.Start (Seconds (0.0));
  
  
  //create application on 9
  uint16_t servPort3 = 9;
  UdpEchoServerHelper echoServer3(servPort3);
  ApplicationContainer serverApps3;
  serverApps3 = echoServer3.Install (c.Get(11));
  serverApps3.Start(Seconds(0.0));
  
  ApplicationContainer clientApps3;
  
  UdpEchoClientHelper echoClient3(i11i7.GetAddress(0), servPort3);
  echoClient3.SetAttribute ("Interval", TimeValue (Seconds (0.00432)));
  echoClient3.SetAttribute ("PacketSize", UintegerValue (sendSize));
  clientApps3 = echoClient3.Install (c.Get (9));  
  clientApps3.Start (Seconds (0.0));
   
  
  
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));
  Simulator::Schedule(Seconds(100000), &Throughput);
  
   //AsciiTraceHelper asc;
   //p2p.EnableAsciiAll(asc.CreateFileStream("my.tr"));
   //p2p.EnablePcapAll("my-udp", false);
  
  Simulator::Stop(Seconds(100000));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
