#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include <ctime>
using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Alfec Random Topology");

uint32_t m_bytesTotal = 0;

static void
ReceivedPacket(Ptr<const Packet> p){

m_bytesTotal += p->GetSize () - 8; 
NS_LOG_UNCOND( " Total Bytes received --------------------------------------------------------------- " << m_bytesTotal);

}

int
main (int argc, char *argv[]){
  
  srand(time(NULL));
  
  	SeedManager::SetSeed (1);  // Changes seed from default of 1 to 3
	SeedManager::SetRun (2);
  
  uint64_t nTransLen = 1024000;
  uint32_t nBlocks = 1;
  uint32_t sendSize = 1000;
  uint32_t numNodes = 10;
  uint32_t numLinks = 20;
  uint32_t numSrc = 3;
  
   CommandLine cmd;
   cmd.AddValue("nTransLen", " Length of data to transfer ", nTransLen);
   cmd.AddValue("nBlocks", " Number of blocks data divided into ", nBlocks);
   cmd.AddValue("numNodes", "Number of nodes in the topology", numNodes);
   cmd.AddValue("numLinks", "Number of links in the topology", numLinks);
   cmd.AddValue("numSrc", "Number of sources", numSrc);
   cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
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
       
  NS_LOG_INFO("Create Link Between Nodes");
  
  NS_LOG_INFO("Enable Global Routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  
    
  uint16_t servPort = 19;
  
  /*for (uint32_t i = 0; i < numNodes; ++i ){
	  
	  UdpEchoServerHelper echoServer(servPort);
	  ApplicationContainer serverApps;
	  serverApps = echoServer.Install (c.Get(i));
	  serverApps.Start(Seconds(0.0));
  }*/
  
  uint32_t i = 0,j = 0, k =0;
  
  //Setting up a network
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
  		}
  }
  
  while(k < numSrc){
	  		
	  	uint32_t src = rand() % numNodes;
	  	uint32_t dest = rand() % numNodes;
		std::cout << "Source and Destination " << src << "\t" << dest << std::endl;
			
  		if(src != dest)
  		{
  			UdpEchoServerHelper echoServer(servPort);
		  	ApplicationContainer serverApps;
		  	serverApps = echoServer.Install (c.Get(dest));
		  	serverApps.Start(Seconds(0.0));
  			
  			Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            x->SetAttribute ("Min", DoubleValue (0));
            x->SetAttribute ("Max", DoubleValue (1));
            double rn = x->GetValue ();
  			
  			Ptr<Node> n = c.Get(dest);
	  		Ptr<Ipv4> ipv = n->GetObject<Ipv4> ();
	  		Ipv4InterfaceAddress ipv4_inter = ipv->GetAddress(1,0); //Parameters GetAddress(interface, addressIndex)
	  		Ipv4Address ip_addr = ipv4_inter.GetLocal();
	  		UdpEchoClientHelper echoClient(ip_addr, servPort);
			echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001686)));
			echoClient.SetAttribute ("PacketSize", UintegerValue (sendSize));
			echoClient.SetAttribute ("TransferLength", UintegerValue(nTransLen));
			echoClient.SetAttribute ("NumOfBlocks", UintegerValue(nBlocks));
			echoClient.SetAttribute ("AppendOverhead", UintegerValue(25 + (rand() % (35 - 25 + 1))));
			ApplicationContainer clientApps;
			clientApps = echoClient.Install (c.Get (src));  
			clientApps.Start (Seconds(rn));
		}
		++k;
  }
  
  
  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&ReceivedPacket));
  
   AsciiTraceHelper asc;
   p2p.EnableAsciiAll(asc.CreateFileStream("rand_topo.tr"));
   //p2p.EnablePcapAll("udp_3pair_1_10", true);
   
   //AnimationInterface anim("animation.xml");
   //for (uint32_t k = 0; k < numNodes; ++k){
   //anim.SetConstantPosition(c.Get(k), k+3, 10);
   //}
   
   
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
