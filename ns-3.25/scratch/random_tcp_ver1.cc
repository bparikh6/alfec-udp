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
  uint64_t maxBytes = 1024000;
  uint32_t sendSize = 1000;
  uint32_t numNodes = 10;
  uint32_t numLinks = 20;
  uint32_t numSrc = 3;

static void
ReceivedPacket(Ptr<const Packet> p, const Address & addr){

m_bytesTotal += p->GetSize ();
NS_LOG_UNCOND( "Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);
if(m_bytesTotal==(numSrc*maxBytes)){
std::cout << "Stop Time " << Simulator::Now ().GetSeconds () << std::endl;
}
}


int
main (int argc, char *argv[]){
  
  srand(time(NULL));
  
  CommandLine cmd;
  cmd.AddValue("maxBytes", " Length of data to transfer ", maxBytes);
  cmd.AddValue("numNodes", "Number of nodes in the topology", numNodes);
  cmd.AddValue("numLinks", "Number of links in the topology", numLinks);
  cmd.AddValue("numSrc", "Number of sources", numSrc);
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
       
   
  NS_LOG_INFO("Enable Global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
    
  uint16_t sinkPort = 19;
  
/*  for (uint32_t i = 0; i < numNodes; ++i ){
	  
   PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
   ApplicationContainer sinkApps = packetSinkHelper.Install(c.Get (i));
   sinkApps.Start(Seconds(0.0));
   
  }*/
  
  std::cout<< "Number of Nodes " << numNodes << std::endl;
  std::cout<< "Number of Links " << numLinks << std::endl;
  
  uint32_t i = 0, j = 0, k = 0;
  
  while(j < numLinks)
  {
  uint32_t n1 = rand() % numNodes;
  uint32_t n2 = rand() % numNodes;
  		
  		if (n1 != n2)
  		{
	  		NodeContainer n_links = NodeContainer(c.Get(n1), c.Get(n2));
	  		NetDeviceContainer n_devices = p2p.Install(n_links); 
	  		std::cout << n1 << "\t" << n2 << std::endl;
	  		ipv4.Assign(n_devices);
	  		ipv4.NewNetwork ();
	  		++i;
	  		++j;
	  		
  		}
  		
  		else{
  			std::cout << "No Link on the same node" << std::endl;
  			//--j;
  		}
  }
  
  	while(k < numSrc)
  	{
		  		
		uint32_t src = rand() % numNodes;
		uint32_t dest = rand() % numNodes;
		std::cout << "Src and Dest" << src << "\t" << dest << std::endl;
		
		if (src!=dest)
		{
			PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
	   		ApplicationContainer sinkApps = packetSinkHelper.Install(c.Get (dest));
	   		sinkApps.Start(Seconds(0.0));
		
			Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            x->SetAttribute ("Min", DoubleValue (0));
            x->SetAttribute ("Max", DoubleValue (1));
            double rn = x->GetValue ();
            
			Ptr<Node> n = c.Get(dest);
			Ptr<Ipv4> ipv = n->GetObject<Ipv4> ();
			Ipv4InterfaceAddress ipv4_inter = ipv->GetAddress(1,0);
			Ipv4Address ip_addr = ipv4_inter.GetLocal();
			BulkSendHelper bulkSend ("ns3::TcpSocketFactory", InetSocketAddress (ip_addr, sinkPort));
			bulkSend.SetAttribute("MaxBytes", UintegerValue (maxBytes));
			bulkSend.SetAttribute("SendSize", UintegerValue (1000));
			ApplicationContainer sourceApps = bulkSend.Install(c.Get(src));
			sourceApps.Start(Seconds(rn));
			++k;
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

// ./waf --run "scratch/random_tcp --maxBytes=102400 --numNodes=10 --numLinks=20 --numSrc=5" >> log.txt 2>&1
