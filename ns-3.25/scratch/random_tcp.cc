#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include <ctime>
using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Random Topology TCP");

uint32_t m_bytesTotal = 0;

static void
ReceivedPacket(Ptr<const Packet> p){

m_bytesTotal += p->GetSize () - 8; 
NS_LOG_UNCOND( " Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);

}

int
main (int argc, char *argv[]){
  
  srand(time(NULL));
   
  uint64_t maxBytes = 1024000;
  uint32_t sendSize = 1000;
  uint32_t numNodes = 10;
  uint32_t numLinks = 3;
  
  CommandLine cmd;
   cmd.AddValue("maxBytes", " Length of data to transfer ", maxBytes);
   //cmd.AddValue("nErrorRate", " Error Rate ", nErrorRate);
   cmd.Parse (argc, argv);
 
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("TcpSocketBase", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1000));

  
  NS_LOG_INFO("Create Nodes");
  NodeContainer c;
  c.Create(numNodes);
  
  NS_LOG_INFO("Create P2P Link Attributes");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", StringValue("1000"));
  
  NS_LOG_INFO("Create Internet Stack To Nodes");
  InternetStackHelper internet;
  internet.Install(NodeContainer::GetGlobal());
  
  NS_LOG_INFO("Asssign Addresses To Nodes");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
       
  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  //em->SetAttribute ("ErrorRate", DoubleValue (nErrorRate));
  //d5d6.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 
  NS_LOG_INFO("Create Link Between Nodes");
  
  NS_LOG_INFO("Enable Global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
    
  uint16_t sinkPort = 19;
  
  for (uint32_t i = 0; i < numNodes; ++i ){
	  
	  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
   ApplicationContainer sinkApps = packetSinkHelper.Install(c.Get (2));
   sinkApps.Start(Seconds(0.0));
  }
  
  uint32_t i = 0,j = 0;
  
  while(i < numNodes && j < numLinks)
  {
  uint32_t n1 = rand() % 10;
  uint32_t n2 = rand() % 10;
  		
  		if (n1 != n2)
  		{
	  		NodeContainer n_links = NodeContainer(c.Get(n1), c.Get(n2));
	  		NetDeviceContainer n_devices = p2p.Install(n_links); 
	  		std::cout << n1 << "\t" << n2 << std::endl;
	  		ipv4.Assign(n_devices);
	  		ipv4.NewNetwork ();
	  		++i;
	  		++j;
	  		Ptr<Node> n = c.Get(n2);
	  		Ptr<Ipv4> ipv = n->GetObject<Ipv4> ();
	  		Ipv4InterfaceAddress ipv4_inter = ipv->GetAddress(1,0);
	  		Ipv4Address ip_addr = ipv4_inter.GetLocal();
	  		BulkSendHelper bulkSend ("ns3::TcpSocketFactory", InetSocketAddress (ip_addr, sinkPort));
   bulkSend.SetAttribute("MaxBytes", UintegerValue (maxBytes));
    bulkSend.SetAttribute("SendSize", UintegerValue (1000));
   ApplicationContainer sourceApps = bulkSend.Install(c.Get(0));
   sourceApps.Start(Seconds(rand()%6));
	  		
  		}
  		
  		else{
  			std::cout << "No Link on the same node" << std::endl;
  		}
  }
  
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));
  
   AsciiTraceHelper asc;
   //p2p.EnableAsciiAll(asc.CreateFileStream("rand_topo.tr"));
   //p2p.EnablePcapAll("udp_3pair_1_10", true);
   
   //AnimationInterface anim("animation.xml");
   //for (uint32_t k = 0; k < numNodes; ++k){
   //anim.SetConstantPosition(c.Get(k), k+3, 10);
   //}
   
   
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
