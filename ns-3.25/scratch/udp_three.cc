#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include <ctime>
using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Dumbbell_three");

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
}


int
main (int argc, char *argv[]){
  
  srand(time(NULL));
   
  uint64_t nTransLen = 1024000;
  uint32_t nBlocks = 1;
  double nErrorRate = 0.01;
  uint32_t sendSize = 1000;
  
   CommandLine cmd;
   cmd.AddValue("nTransLen", " Length of data to transfer ", nTransLen);
   cmd.AddValue("nBlocks", " Number of blocks data divided into ", nBlocks);
   cmd.AddValue("nErrorRate", " Error Rate ", nErrorRate);
    cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  NodeContainer c;
  c.Create(10);
  
  NodeContainer n0n4 = NodeContainer (c.Get(0), c.Get(4));
  NodeContainer n1n4 = NodeContainer (c.Get(1), c.Get(4));
  NodeContainer n8n4 = NodeContainer (c.Get(8), c.Get(4));
  NodeContainer n2n7 = NodeContainer (c.Get(2), c.Get(7));
  NodeContainer n3n7 = NodeContainer (c.Get(3), c.Get(7));
  NodeContainer n9n7 = NodeContainer (c.Get(9), c.Get(7));
  NodeContainer n4n5 = NodeContainer (c.Get(4), c.Get(5));
  NodeContainer n5n6 = NodeContainer (c.Get(5), c.Get(6));
  NodeContainer n6n7 = NodeContainer (c.Get(6), c.Get(7));
  
  //Install Iternet Stack
  InternetStackHelper internet;
  internet.Install(c);
  
  
  NS_LOG_INFO("Create Channels");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", StringValue("1000"));
  
  NetDeviceContainer d0d4 = p2p.Install(n0n4);
  NetDeviceContainer d1d4 = p2p.Install(n1n4);
  NetDeviceContainer d8d4 = p2p.Install(n8n4);
  NetDeviceContainer d2d7 = p2p.Install(n2n7);
  NetDeviceContainer d3d7 = p2p.Install(n3n7);
  NetDeviceContainer d9d7 = p2p.Install(n9n7);
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
 
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i7 = ipv4.Assign(d2d7);
  
  ipv4.SetBase("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i7 = ipv4.Assign(d3d7);
  
  ipv4.SetBase("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i7 = ipv4.Assign(d9d7);
  
  ipv4.SetBase("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
  
  ipv4.SetBase("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);
  
  ipv4.SetBase("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i6i7 = ipv4.Assign(d6d7);
     
  NS_LOG_INFO("Enable global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
  
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  em->SetAttribute ("ErrorRate", DoubleValue (nErrorRate));
  d5d6.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 
  NS_LOG_INFO("Create Applications");
  
  //create applicaiton on 0
  uint16_t servPort = 90;
  UdpEchoServerHelper echoServer(servPort);
  ApplicationContainer serverApps;
  serverApps = echoServer.Install (c.Get(2));
  serverApps.Start(Seconds(0.0));
  
  ApplicationContainer clientApps;
  UdpEchoClientHelper echoClient(i2i7.GetAddress(0), servPort);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001792)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (sendSize));
  echoClient.SetAttribute ("TransferLength", UintegerValue(nTransLen));
  echoClient.SetAttribute ("NumOfBlocks", UintegerValue(nBlocks));
  echoClient.SetAttribute ("AppendOverhead", UintegerValue(25 + (rand() % (35 - 25 + 1))));
  clientApps = echoClient.Install (c.Get (0));  
  clientApps.Start (Seconds(rand() % 6));
  
  //create application on 1
  uint16_t servPort1 = 8000;
  UdpEchoServerHelper echoServer1(servPort1);
  ApplicationContainer serverApps1;
  serverApps1 = echoServer1.Install (c.Get(3));
  serverApps1.Start(Seconds(0.0));
  

  ApplicationContainer clientApps1;
  UdpEchoClientHelper echoClient1(i3i7.GetAddress(0), servPort1);
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (0.001686)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (sendSize));
  echoClient1.SetAttribute ("TransferLength", UintegerValue(nTransLen));
  echoClient1.SetAttribute ("NumOfBlocks", UintegerValue(nBlocks));
  echoClient1.SetAttribute ("AppendOverhead", UintegerValue(25 + (rand() % (35 - 25 + 1))));
  clientApps1 = echoClient1.Install (c.Get (1));  
  clientApps1.Start (Seconds(rand() % 6));
  
  //create application on 8
  uint16_t servPort2 = 8080;
  UdpEchoServerHelper echoServer2(servPort2);
  ApplicationContainer serverApps2;
  serverApps2 = echoServer2.Install (c.Get(9));
  serverApps2.Start(Seconds(0.0));
  
  ApplicationContainer clientApps2;
  UdpEchoClientHelper echoClient2(i9i7.GetAddress(0), servPort2);
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (0.001692)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (sendSize));
  echoClient2.SetAttribute ("TransferLength", UintegerValue(nTransLen));
  echoClient2.SetAttribute ("NumOfBlocks", UintegerValue(nBlocks));
  echoClient2.SetAttribute ("AppendOverhead", UintegerValue(25 + (rand() % (35 - 25 + 1))));
  clientApps2 = echoClient2.Install (c.Get (8));  
  clientApps2.Start (Seconds(rand() % 6));
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));
  Simulator::Schedule(Seconds(1000000), &Throughput);
  
   AsciiTraceHelper asc;
   //p2p.EnableAsciiAll(asc.CreateFileStream("udp_3pair_0%.tr"));
   //p2p.EnablePcapAll("udp_3pair_1_10", true);
   
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
