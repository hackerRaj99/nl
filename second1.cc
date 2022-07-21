/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BusTopology");

int
main (int argc, char *argv[])
{
  uint32_t nCsma = 3;
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of  \"extra\" CSMA nodes/devices",nCsma);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);
  
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  
  
  
  nCsma = nCsma == 0 ? 1 : nCsma;
  
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get(1));
  csmaNodes.Create (nCsma);
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
  
  stack.Install (csmaNodes);
  
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
  
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
  
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
   
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);
  
  AnimationInterface anim("Pratik.xml");
  anim.SetConstantPosition(p2pNodes.Get(0),6,12);
  anim.SetConstantPosition(csmaNodes.Get(0),30,12);
  anim.SetConstantPosition(csmaNodes.Get(1),40.0,12.0);
  anim.SetConstantPosition(csmaNodes.Get(2),50.0,12.0);
  anim.SetConstantPosition(csmaNodes.Get(3),60.0,12.0);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
