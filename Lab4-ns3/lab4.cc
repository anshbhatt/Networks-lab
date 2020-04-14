/**
  * Network topology:
    H1                          H3
       \                      /
        \                   /
         R1 ------------- R2
        /                   \
      /                      \
    H2                        H4
    H1, H3 - TCP host
    H2, H4 - UDP host
    (H2-H4): CBR (constant bit rate) and (H1, H3): ftp
    host-router: 80Mbps 20ms
    router-router: 30Mbps, 100ms
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

typedef uint32_t uint;

NS_LOG_COMPONENT_DEFINE("lab4");


int main(int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  LogComponentEnable("lab4", LOG_LEVEL_INFO);

  // Some info about tcp connection:
  uint32_t max_bytes = 0;
  uint32_t packet_size = 100; // base packet size, after every loop we will increase this:
  //uint32_t max_packets = 0;    // max packets hosts can sends;
  uint32_t sim_count = 0;      // no of times simulation needs to be ran (for various packet size)
  uint32_t run_time = 0;  // time interval for sim.
  std::string prot = "Highspeed";
  bool simultaneously = false;

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  //bool enableFlowMonitor = true;

  NS_LOG_INFO("Prabhu");

  cmd.AddValue("max_bytes", "Maximum no of bytes host can sends", max_bytes);
  cmd.AddValue("prot", "Protocol needs to be used TcpVegas/TcpHighSpeed?", prot);
  cmd.AddValue("packetsize", "Starting packet size", packet_size);
  cmd.AddValue("run_time", "Run time", run_time);
  cmd.AddValue("sim_count", "No of times simulation needs to run?", sim_count);
  cmd.AddValue("simultaneously", "to run together or not", simultaneously);

  cmd.Parse(argc, argv);

  // set protocol needs to be used:
  if (prot == "TcpVegas")
  {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVegas::GetTypeId()));
  }
  else if (prot == "TcpHighSpeed")
  {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHighSpeed::GetTypeId()));
  }
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpScalable::GetTypeId()));

  Gnuplot2dDataset dataset_udp;
  Gnuplot2dDataset dataset_tcp;
  Gnuplot2dDataset dataset_udp_delay;
  Gnuplot2dDataset dataset_tcp_delay;

  for (uint i = 0; i < sim_count; i++)
  {
    packet_size = packet_size + 100;

    NS_LOG_INFO("Create nodes.");
    NodeContainer c;
    c.Create(6);

    NodeContainer h1r1 = NodeContainer(c.Get(0), c.Get(2));
    NodeContainer h2r1 = NodeContainer(c.Get(1), c.Get(2));
    NodeContainer r1r2 = NodeContainer(c.Get(2), c.Get(3));
    NodeContainer h3r2 = NodeContainer(c.Get(4), c.Get(3));
    NodeContainer h4r2 = NodeContainer(c.Get(5), c.Get(3));

    InternetStackHelper internet;
    internet.Install(c);

    uint max_queue_size_1 = (20*80000)/packet_size;
    uint max_queue_size_2 = (100*30000)/packet_size;

    // We create the channels first without any IP addressing information
    NS_LOG_INFO("Create channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("20ms"));
    p2p.SetQueue ("ns3::DropTailQueue",
              "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, max_queue_size_1)));

    NetDeviceContainer d_h1r1 = p2p.Install(h1r1);
    NetDeviceContainer d_h2r1 = p2p.Install(h2r1);
    NetDeviceContainer d_h3r2 = p2p.Install(h3r2);
    NetDeviceContainer d_h4r2 = p2p.Install(h4r2);

    p2p.SetDeviceAttribute("DataRate", StringValue("30Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("100ms"));
    p2p.SetQueue ("ns3::DropTailQueue",
              "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, max_queue_size_2)));
    NetDeviceContainer d_r1r2 = p2p.Install(r1r2);

    // Later, we add IP addresses.
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ip_h1r1 = ipv4.Assign(d_h1r1);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ip_h2r1 = ipv4.Assign(d_h2r1);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ip_r1r2 = ipv4.Assign(d_r1r2);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer ip_h3r2 = ipv4.Assign(d_h3r2);

    ipv4.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer ip_h4r2 = ipv4.Assign(d_h4r2);

    // Print assigned IPs:
    std::cout<<"IP of H1: "<<ip_h1r1.GetAddress(0)<<"     "<<"IP of H2: "<<ip_h2r1.GetAddress(0)<<std::endl;
    std::cout<<"IP of H3: "<<ip_h3r2.GetAddress(0)<<"     "<<"IP of H4: "<<ip_h4r2.GetAddress(0)<<std::endl;
    std::cout<<"IP of R1: "<<ip_r1r2.GetAddress(0)<<"     "<<"IP of R2: "<<ip_r1r2.GetAddress(1)<<std::endl;

    // updated upto here.

    // Create router nodes, initialize routing database and set up the routing
    // tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //printing routing tables for the all the nodes in the container
    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (2), routingStream);


    // Create the OnOff application to send UDP datagrams: CBR Traffic
    NS_LOG_INFO("Create Applications.");
    uint16_t port = 9; // Discard port (RFC 863)
    OnOffHelper onoff("ns3::UdpSocketFactory",
                      Address(InetSocketAddress(ip_h4r2.GetAddress(0), port))); // here addresse of h4:

    onoff.SetAttribute ("PacketSize", UintegerValue (packet_size));
    ApplicationContainer udp_apps = onoff.Install(c.Get(1)); // H2
    if(simultaneously==false)
    {
      udp_apps.Start (Seconds ( (0.0+(10*i))*run_time  ) );
      udp_apps.Stop (Seconds ((5.0+(10* i))*run_time) );
    }
    else
    {
      udp_apps.Start (Seconds ( (0.0+(10*i))*run_time  ) );
      udp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }

    // Create a packet sink to receive these packets
    PacketSinkHelper udp_sink("ns3::UdpSocketFactory",
                          Address(InetSocketAddress(Ipv4Address::GetAny(), port))); // can receive from any
    udp_apps = udp_sink.Install(c.Get(5));                                                  // H4
   if(simultaneously==false)
    {
      udp_apps.Start (Seconds ((0.0+(10*i))*run_time) );
      udp_apps.Stop (Seconds ((5.0+(10*i))*run_time) );
    }
    else
    {
      udp_apps.Start (Seconds ((0.0+(10*i))*run_time) );
      udp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }

    /*
     * FTP Traffic using TCP
     *
    **/
    port = 12344;

    BulkSendHelper server ("ns3::TcpSocketFactory", InetSocketAddress(ip_h3r2.GetAddress(0), port));
    server.SetAttribute ("MaxBytes", UintegerValue (max_bytes));
    server.SetAttribute ("SendSize", UintegerValue (packet_size));
    ApplicationContainer tcp_apps = server.Install (h1r1.Get(0)); // H1

    if(simultaneously==false)
    {
        tcp_apps.Start (Seconds ((5.0+(10*i))*run_time) );
        tcp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }
    else
    {
        tcp_apps.Start (Seconds ((0.0+(10*i))*run_time) );
        tcp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }

    // packet sink to receive ftp packets
    PacketSinkHelper tcp_sink("ns3::TcpSocketFactory",
                          Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    tcp_apps = tcp_sink.Install(h3r2.Get(0)); // H3
    if(simultaneously==false)
    {
        tcp_apps.Start (Seconds ((5.0+(10*i))*run_time) );
        tcp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }
    else
    {
      tcp_apps.Start (Seconds ((0.0+(10*i))*run_time) );
        tcp_apps.Stop (Seconds ((10.0+(10*i))*run_time) );
    }


    NS_LOG_INFO("Run Simulation.");

    // Upto here: (Topology created. bhai siddhant ab tu data extract karke plot kar dena please bhai)
    /*
      **************************
        LOGGING of PARAMETERS
      **************************
    */

    Ptr<FlowMonitor> flowmon;
    FlowMonitorHelper flowmonHelper;
    flowmon = flowmonHelper.InstallAll();
    Simulator::Stop(Seconds((10+(10*i))*run_time) );
    Simulator::Run();
    //flowmon->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats();

    double throughput_udp;
    double throughput_tcp;
    double delay_udp;
    double delay_tcp;

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout<<t.sourceAddress<<"\n";
      std::cout<<t.destinationAddress<<"\n";

      if(t.sourceAddress == "10.1.2.1") {
        throughput_udp = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000;
        delay_udp = i->second.delaySum.GetSeconds()/(i->second.rxPackets) ;

        dataset_udp.Add (packet_size,throughput_udp);
        dataset_udp_delay.Add (packet_size,delay_udp);

        std::cout << "UDP Flow over CBR " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "Tx Bytes:" << i->second.txBytes << "\n";
        std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "Rx Bytes:" << i->second.rxBytes << "\n";
        std::cout << "Net Packet Lost: " << i->second.lostPackets << "\n";
        std::cout << "Lost due to droppackets: " << i->second.packetsDropped.size() << "\n";
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/0/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/1/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/2/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/3/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/4/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/5/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        std::cout << "Delay: " << i->second.delaySum.GetSeconds() << std::endl;
        std::cout << "Mean Delay: " << i->second.delaySum.GetSeconds()/(i->second.rxPackets) << std::endl;
        std::cout << "Offered Load: " << i->second.txBytes * 8.0 / (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
        std::cout << "Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
        std::cout << "Mean jitter:" << i->second.jitterSum.GetSeconds () / (i->second.rxPackets - 1) << std::endl;
        std::cout<<std::endl;

      }
      else if(t.sourceAddress == "10.1.1.1") {
        throughput_tcp = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000;
        delay_tcp = i->second.delaySum.GetSeconds()/(i->second.rxPackets);

        dataset_tcp.Add (packet_size,throughput_tcp);
        dataset_tcp_delay.Add(packet_size,delay_tcp);

        std::cout << prot <<" Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "Tx Bytes:" << i->second.txBytes << "\n";
        std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "Rx Bytes:" << i->second.rxBytes << "\n";
        std::cout << "Net Packet Lost: " << i->second.lostPackets << "\n";
        std::cout << "Lost due to droppackets: " << i->second.packetsDropped.size() << "\n";
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/0/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/1/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/2/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/3/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/4/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        // std::cout << "Max throughput: " << mapMaxThroughput["/NodeList/5/$ns3::Ipv4L3Protocol/Rx"] << std::endl;
        std::cout << "Delay: " << i->second.delaySum.GetSeconds() << std::endl;
        std::cout << "Mean Delay: " << i->second.delaySum.GetSeconds()/(i->second.rxPackets) << std::endl;
        std::cout << "Offered Load: " << i->second.txBytes * 8.0 / (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
        std::cout << "Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
        std::cout << "Mean jitter:" << i->second.jitterSum.GetSeconds () / (i->second.rxPackets - 1) << std::endl;
        std::cout<<std::endl;
      }
    }
    Simulator::Destroy();
  }
  // std::cout << dataset_tcp << endl;
  std::string simultaneously_str = std::to_string(simultaneously);
  std::string fileNameWithNoExtension = prot+simultaneously_str;
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = prot + "vs UDP throughput";

  std::string fileNameWithNoExtension_delay = prot+"_delay"+simultaneously_str;
  std::string graphicsFileName_delay        = fileNameWithNoExtension_delay + ".png";
  std::string plotFileName_delay            = fileNameWithNoExtension_delay + ".plt";
  std::string plotTitle_delay               = prot + "vs UDP delay";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  Gnuplot plot_delay (graphicsFileName_delay);

  plot.SetTitle (plotTitle);
  plot_delay.SetTitle (plotTitle_delay);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");
  plot_delay.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("Packet Size(in Bytes)", "Throughput Values(in mbps)");
  plot_delay.SetLegend ("Packet Size(in Bytes)", "Delay(in s)");

  // Set the range for the x axis.
  // plot.AppendExtra ("set xrange [-6:+6]");

  // Instantiate the dataset, set its title, and make the points be
  // plotted along with connecting lines.
  dataset_tcp.SetTitle ("Throughput FTP over TCP");
  dataset_tcp.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset_udp.SetTitle ("Throughput CBR over UDP");
  dataset_udp.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  dataset_tcp_delay.SetTitle ("Delay FTP over TCP");
  dataset_tcp_delay.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset_udp_delay.SetTitle ("Delay CBR over UDP");
  dataset_udp_delay.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  // double x;
  // double y;


  // Add the dataset to the plot.
  plot.AddDataset (dataset_tcp);
  plot.AddDataset (dataset_udp);

  plot_delay.AddDataset (dataset_udp_delay);
  plot_delay.AddDataset (dataset_tcp_delay);

  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();

  std::ofstream plotFile_delay (plotFileName_delay.c_str());
  plot_delay.GenerateOutput (plotFile_delay);
  plotFile_delay.close ();

  return 0;
}
