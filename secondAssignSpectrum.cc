/*
Costruire una rete formata da almeno 4 nodi wireless: 2 Access Points  (AP) e 2 nodi (STA). 
- I due AP devono essere "vicini" (spazialmente) in modo da subire mutua interferenza. 
    Impostare i 2 AP in modo che usino 2 canali WiFi tali da generare mutua interferenza.
- Si associno i nodi STA a uno dei due AP, in modo che parte siano associati a un AP, e parte all'altro.
- Si connettano i due AP tramite una rete CSMA o P2P. In questo caso si provveda a mettere le necessarie regole di routing.
- Si generi traffico da o per i nodi STA (a piacere).

YansWiFiChannel
SpectrumWiFiChannel
*/

// STA              STA
//  |                 |
// n0                n1  
//  |                 |
// n2 -------------- n3
//  |                 |
//  *                 *
// AP                AP

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/spectrum-channel.h"
#include "ns3/spectrum-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include <chrono>




using namespace ns3;

NS_LOG_COMPONENT_DEFINE("secondAssignSpectrum");

void CalculateThroughput (Ptr<FlowMonitor> monitor, FlowMonitorHelper &flowmon)
{
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow ID: " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 1024 / 1024 << " Mbps\n";
      std::cout << "  Packet Loss: " << i->second.lostPackets << "\n";
    }

  Simulator::Schedule(Seconds (1.0), &CalculateThroughput, monitor, std::ref(flowmon));
} 

int main (int argc, char *argv[])
{
  auto start= std::chrono::high_resolution_clock::now();
    // Variabili di tipo uint32
  uint32_t nAP = 2;
  uint32_t nSTA = 2;
  uint32_t serv = 1;
  CommandLine cmd(__FILE__);
  cmd.Parse (argc, argv);

    // Creazione di 2 AP e 2 STA
  NodeContainer wifiApNodes;
  wifiApNodes.Create(nAP); // 2 AP
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(nSTA); // 2 STA
   
  NodeContainer serverNode;
  serverNode.Create(serv);
//***********************************************************************************************

    // Impostazione dei canali WiFi per generare mutua interferenza
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);


//**********************************************************************************************
    SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default();
    channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
    channelHelper.AddSpectrumPropagationLoss("ns3::ConstantSpectrumPropagationLossModel");
    Ptr<SpectrumChannel> channel = channelHelper.Create();

    SpectrumWifiPhyHelper spectrumPhy;
    spectrumPhy.SetChannel(channel);
    spectrumPhy.SetErrorRateModel("ns3::NistErrorRateModel");


//*********************************************************************************************

    // Canale per AP1 e STA1

  //auto channel = wifiChannel.Create();
  //wifiPhy1.SetChannel(channel); 
  spectrumPhy.Set("ChannelSettings", StringValue("{1, 20, BAND_2_4GHZ, 0}"));

  WifiMacHelper wifiMac1;
  Ssid ssid1 = Ssid("network-1");
  wifiMac1.SetType("ns3::StaWifiMac", 
                      "Ssid", SsidValue(ssid1), 
                      "ActiveProbing", BooleanValue(false));
  NetDeviceContainer staDevices1 = wifi.Install(spectrumPhy, wifiMac1, wifiStaNodes.Get(0));
  wifiMac1.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid1));
  NetDeviceContainer apDevices1 = wifi.Install(spectrumPhy, wifiMac1, wifiApNodes.Get(0));

  // Canale per AP2 e STA2
  //YansWifiPhyHelper wifiPhy2;
  //wifiPhy2.SetChannel(channel);
  spectrumPhy.Set("ChannelSettings", StringValue("{5, 20, BAND_2_4GHZ, 0}")); 
    
  WifiMacHelper wifiMac2;
  Ssid ssid2 = Ssid("network-2");
  wifiMac2.SetType("ns3::StaWifiMac",
                  "Ssid", SsidValue(ssid2),
                  "ActiveProbing", BooleanValue(false));
  NetDeviceContainer staDevices2 = wifi.Install(spectrumPhy, wifiMac2, wifiStaNodes.Get(1));
  wifiMac2.SetType("ns3::ApWifiMac",
                  "Ssid", SsidValue(ssid2));
  NetDeviceContainer apDevices2 = wifi.Install(spectrumPhy, wifiMac2, wifiApNodes.Get(1));


  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate",StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay",TimeValue(NanoSeconds(6560)));
  NodeContainer csmaNodes;
  csmaNodes.Add(serverNode.Get(0));
  csmaNodes.Add(wifiApNodes.Get(0));
  csmaNodes.Add(wifiApNodes.Get(1));
  NetDeviceContainer csmaDevices = csma.Install(csmaNodes);

//*********************************************************************************************


  InternetStackHelper stack;
  Ipv4NixVectorHelper nix;
  stack.SetRoutingHelper(nix);
  stack.Install(wifiStaNodes);
  stack.Install(wifiApNodes);
  stack.Install(serverNode);

  Ipv4AddressHelper address;

//**********************************************************************************************

  // Assegnazione degli indirizzi IP agli STA
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfaces1 = address.Assign(staDevices1);
  Ipv4InterfaceContainer apInterfaces1 = address.Assign(apDevices1);

  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfaces2 = address.Assign(staDevices2);
  Ipv4InterfaceContainer apInterfaces2 = address.Assign(apDevices2);

  // Assegnazione degli indirizzi IP per la connessione tra AP1 e AP2
  address.SetBase("10.1.3.0", "255.255.255.0");
  //address.Assign(csmaDevices);
  Ipv4InterfaceContainer csmaDevicesInt =address.Assign(csmaDevices);

//**********************************************************************************************
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(5.0),
                                  "GridWidth",
                                  UintegerValue(2),
                                  "LayoutType",
                                  StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiStaNodes);
  mobility.Install(wifiApNodes);
  mobility.Install(serverNode);
//**********************************************************************************************

// Configurazione del server Echo
  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(serverNode.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(20.0));


  // Configurazione del client Echo
  UdpEchoClientHelper echoClient1(csmaDevicesInt.GetAddress(2), 9);
  echoClient1.SetAttribute("MaxPackets", UintegerValue(0));
  echoClient1.SetAttribute("Interval", TimeValue(Seconds(0.001)));
  echoClient1.SetAttribute("PacketSize", UintegerValue(4096));

  ApplicationContainer clientApps1 = echoClient1.Install(wifiStaNodes.Get(0));
  clientApps1.Start(Seconds(2.0)); // Inizia dopo 2 secondi per garantire che il server sia pronto
  clientApps1.Stop(Seconds(18.0));


  UdpEchoClientHelper echoClient2(csmaDevicesInt.GetAddress(2), 9);
  echoClient2.SetAttribute("MaxPackets", UintegerValue(0));
  echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.001)));
  echoClient2.SetAttribute("PacketSize", UintegerValue(4096));

  ApplicationContainer clientApps2 = echoClient2.Install(wifiStaNodes.Get(1));
  clientApps2.Start(Seconds(2.0)); // Inizia dopo 2 secondi per garantire che il server sia pronto
  clientApps2.Stop(Seconds(18.0));



  //***********************************************************************************************

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  spectrumPhy.EnablePcapAll("secondAssignmentSpectrum");
  csma.EnablePcapAll("secondAssignmentSpectrum");
  // Avvia la simulazione
  Simulator::Stop(Seconds(20.0));
  Simulator::Run();

  monitor->CheckForLostPackets();

  CalculateThroughput(monitor, flowmon);
  monitor->SerializeToXmlFile("flowmon-results.xml", true, true);

  Simulator::Destroy();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
  std::cout << duration.count() << std::endl;

  return 0;
}