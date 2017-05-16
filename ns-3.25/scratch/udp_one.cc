#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("One_Pair");

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
  
  uint32_t sendSize = 1000;
  
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
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
  
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  d4d5.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  
  //create applicaiton on 0
  uint16_t servPort = 8080;
  UdpEchoServerHelper echoServer(servPort);
  ApplicationContainer serverApps;
  serverApps = echoServer.Install (c.Get(5));
  serverApps.Start(Seconds(0.0));
  
  
  UdpEchoClientHelper echoClient(i4i5.GetAddress(1), servPort);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001686)));
  echoClient.SetAttribute("AppendOverhead", UintegerValue(25));
  echoClient.SetAttribute ("PacketSize", UintegerValue (sendSize));
  
  ApplicationContainer clientApps;
  clientApps = echoClient.Install (c.Get (0));  
  clientApps.Start (Seconds (0.0));
  
  
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));
  Simulator::Schedule(Seconds(10000000), &Throughput);
  
   AsciiTraceHelper asc;
   pp.EnableAsciiAll(asc.CreateFileStream("udp_1pair_0%.tr"));
   //pp.EnablePcapAll("udp-1pair", false);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
