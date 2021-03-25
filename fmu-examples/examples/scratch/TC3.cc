#include "ns3/core-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/global-value.h"

#include "ns3/fmi-export-module.h"

#include "ns3/tc3-controller-server.h"
#include "ns3/tc3-controller-client.h"
#include "ns3/tc3-custom-server.h"
#include "ns3/tc3-smartmeter-custom-client.h"
#include "ns3/tc3-helper.h"

#include <iostream>
#include <algorithm>

/*
 Network Topology

 Wifi 192.168.137.0	  		Csma(ethernet) 192.168.147.0

  OLTC SMB SMA  AP(csma(1) / wifiApNodes)
	*	*	*	*
					------------------------------ *Server(Controller)
*/

using namespace ns3;


NS_LOG_COMPONENT_DEFINE( "TC3" );

class TC3CommNetworkFMU : public SimpleEventQueueFMUBase {

public:

	// Define FMI input/output variables and parameters as class members
	fmi2Integer u3_send;	// Input variable associated to SmartMeterA
	fmi2Integer u4_send; // Input variable associated to SmartMeterB
	fmi2Integer ctrl_send;  // Input variable associated to controller

	fmi2Integer ctrl_receive;  // Output variable associated to controller
	fmi2Integer tap_receive; // Output variable associated to the OLTC

	// Variables to hold the nodes' IP addresses
	Ipv4Address smartMeterA_;
	Ipv4Address smartMeterB_;
	Ipv4Address transformer_;

	// Variables are used to keep the delay from each smart meter to the controller
	double SMA_delay_ = 0;
	double SMB_delay_ = 0;
	double ctrl_delay_ = 0;

	double delay_factor_ = 1e0;

	// Define the inputs outputs and parameters of the ns3 simulation
	virtual void initializeSimulation();

	// Define the ns3 simulation that should be run
	virtual void runSimulation(const double& sync_time);

};


void
TC3CommNetworkFMU::initializeSimulation()
{
	// Define FMI integer input variables
	addIntegerInput( u3_send );
	addIntegerInput( u4_send );
	addIntegerInput( ctrl_send );

	// Define FMI integer output variable
	addIntegerOutput( ctrl_receive );
	addIntegerOutput( tap_receive );

}


void
TC3CommNetworkFMU::runSimulation(const double& sync_time)
{
	// Check if any parameter is changed, so we must run a simulation
	if(( u3_send == 0 ) &&( u4_send == 0 ) && ( ctrl_send == 0 ) ) return;

	LogComponentEnable( "TC3OltcCustomServer", LOG_LEVEL_ERROR );
	LogComponentEnable( "TC3SmartmeterCustomClient", LOG_LEVEL_ERROR );
	LogComponentEnable( "TC3ControllerClient", LOG_LEVEL_ERROR );
	LogComponentEnable( "TC3ControllerServer", LOG_LEVEL_ERROR );

	/*
	// This part of code will enable the PyViz tool.

	std::vector<std::string> strVec {"tc3_fmi/ns3", "--SimulatorImplementationType=ns3::VisualSimulatorImpl"};
	char** ptr_str = new char*[strVec.size()];
	for( size_t i=0; i<2; i++ )
	{
		ptr_str[i] = new char[strVec[i].size() + 1];
		std::strcpy(ptr_str[i], strVec[i].c_str());
	}

	CommandLine cmd;
	cmd.Parse(2, ptr_str);
	*/

	// Create the nodes
  	NodeContainer csmaNodes;
	csmaNodes.Create(2);
	CsmaHelper csma;
	csma.SetChannelAttribute( "DataRate", StringValue( "100Mbps" ) );
	//csma.SetChannelAttribute( "Delay", TimeValue( NanoSeconds( 6560 ) ) );
	csma.SetChannelAttribute( "Delay", TimeValue( MilliSeconds( 10 ) ) );

	NodeContainer wifiApNodes = csmaNodes.Get(1);

	NodeContainer wifiStaNodes;
	wifiStaNodes.Create(3);

	// Add network devices to the nodes
	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install( csmaNodes );

	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
	YansWifiPhyHelper phy;
	phy.SetChannel( channel.Create() );
	WifiHelper wifi;
	wifi.SetRemoteStationManager( "ns3::AarfWifiManager" );
	WifiMacHelper mac;
	Ssid ssid = Ssid( "tc3_AP" );
	mac.SetType( "ns3::ApWifiMac", "Ssid", SsidValue( ssid ) );
	NetDeviceContainer apDevices;
	apDevices = wifi.Install( phy, mac, wifiApNodes );

	mac.SetType( "ns3::StaWifiMac", "Ssid", SsidValue( ssid ), "ActiveProbing", BooleanValue( false ) );
	NetDeviceContainer staDevices;
	staDevices = wifi.Install( phy, mac, wifiStaNodes );

	// Add internet stacks to the nodes, so we can define the networks
	InternetStackHelper stack;
	stack.Install( csmaNodes );
	stack.Install( wifiStaNodes );

	// Configure the networks, define IP addresses
	Ipv4AddressHelper address;
	address.SetBase( "192.168.137.0", "255.255.255.0" );
	Ipv4InterfaceContainer csmaInterfaces;

	csmaInterfaces = address.Assign( csmaDevices );
	address.SetBase( "192.168.147.0", "255.255.255.0" );
	Ipv4InterfaceContainer wifiStaInterfaces;
  	address.Assign( apDevices );
	wifiStaInterfaces = address.Assign( staDevices );

	Ptr<Node> node = wifiStaNodes.Get(0);
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
	smartMeterA_ = ipv4->GetAddress(1,0).GetLocal();

	node = wifiStaNodes.Get(1);
	ipv4 = node->GetObject<Ipv4>();
	smartMeterB_ = ipv4->GetAddress(1,0).GetLocal();

	node = wifiStaNodes.Get(2);
	ipv4 = node->GetObject<Ipv4>();
	transformer_ = ipv4->GetAddress(1,0).GetLocal();

	// Place the nodes at the right positions
  	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
	positionAlloc->Add( Vector( 0, 0, 0 ) );
	positionAlloc->Add( Vector( 10, 0, 0 ) );
	mobility.SetPositionAllocator( positionAlloc );
	mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
	mobility.Install( csmaNodes );

	Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator>();
	positionAlloc2->Add( Vector( 20, 0, 0 ) );
	positionAlloc2->Add( Vector( 10, 10, 0 ) );
	positionAlloc2->Add( Vector( 10, -10, 0 ) );
	mobility.SetPositionAllocator( positionAlloc2 );
	mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
	mobility.Install( wifiStaNodes );

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Simulation topology complete, assign applications to the nodes

    // Smartmeters send at the same time data to the server.
	if( ( u3_send !=0 ) ||( u4_send != 0 ) )
    {
		// std::cout << "\n(Debug): SMART METER SEND" << std::endl;
	  	TC3SmartmeterCustomClientHelper smartMeterHlpA( csmaInterfaces.GetAddress(0), 9 );
	  	smartMeterHlpA.SetAttribute( "MaxPackets", UintegerValue(1) );
	  	smartMeterHlpA.SetAttribute( "Interval", TimeValue( Seconds(10.0) ) );
	  	smartMeterHlpA.SetAttribute( "PacketSize", UintegerValue(100) );
	  	ApplicationContainer smartMeterAppA = smartMeterHlpA.Install( wifiStaNodes.Get(0) );
	  	smartMeterAppA.Start( Seconds(1.0) );
	  	smartMeterAppA.Stop( Seconds(10.0) );

	  	TC3SmartmeterCustomClientHelper smartMeterHlpB( csmaInterfaces.GetAddress(0), 9 );
	  	smartMeterHlpB.SetAttribute( "MaxPackets", UintegerValue(1) );
	  	smartMeterHlpB.SetAttribute( "Interval", TimeValue( Seconds(10.0) ) );
	  	smartMeterHlpB.SetAttribute( "PacketSize", UintegerValue(100) );
	  	ApplicationContainer smartMeterAppB = smartMeterHlpB.Install( wifiStaNodes.Get(1) );
	  	smartMeterAppB.Start( Seconds(1.0) );
	  	smartMeterAppB.Stop( Seconds(10.0) );

  		// Controller receives from the smartmeters in this case.
    	TC3ControllerServerHelper controllerHlp(9);
		controllerHlp.SetAttribute( "SmartMeterA_Ipv4Address", AddressValue( smartMeterA_ ) );
		controllerHlp.SetAttribute( "SmartMeterB_Ipv4Address", AddressValue( smartMeterB_ ) );
	  	ApplicationContainer controllerApp = controllerHlp.Install( csmaNodes.Get(0) );
	  	controllerApp.Start( Seconds(0.0) );
	  	controllerApp.Stop( Seconds(10.0) );

		Simulator::Stop( Seconds(10.0) );

		Simulator::Run();

		const TC3ControllerServer& SM = dynamic_cast<const TC3ControllerServer&>( *controllerApp.Get(0) );

		// Retrieve the end to end delay for each smartmeter
   	  	SMA_delay_ = SM.GetEndToEndDelay_SMA();
		SMB_delay_ = SM.GetEndToEndDelay_SMB();
		
		// Add the two events to the event queue.
		if( u3_send != 0 ) addNewEventForMessage( sync_time + delay_factor_ * SMA_delay_, u3_send, &ctrl_receive );
		if( u4_send != 0 ) addNewEventForMessage( sync_time + delay_factor_ * SMB_delay_, u4_send, &ctrl_receive );
	}


    if( ctrl_send != 0 )
	{
		// std::cout << "\n(Debug): CTRL SEND" << std::endl;
	  	// Controller acts as the client in this case
	  	TC3ControllerClientHelper controllerHlp( wifiStaInterfaces.GetAddress(0), 19 );
	  	controllerHlp.SetAttribute( "MaxPackets", UintegerValue(1) );
		ApplicationContainer controllerApp = controllerHlp.Install( csmaNodes.Get(0) );
	  	controllerApp.Start( Seconds(2.0) );
	  	controllerApp.Stop( Seconds(10.0) );

	  	TC3OltcCustomServerHelper OltcHlp(19);
	  	ApplicationContainer OltcApp = OltcHlp.Install( wifiStaNodes.Get(0) );
	  	OltcApp.Start( Seconds(0.0) );
	  	OltcApp.Stop( Seconds(10.0) );

		Simulator::Stop( Seconds(10.0) );

		Simulator::Run();

		const TC3OltcCustomServer& oltc_srv = dynamic_cast<const TC3OltcCustomServer&>( *OltcApp.Get(0) );
		ctrl_delay_ = oltc_srv.GetEndToEndDelay();

		addNewEventForMessage( sync_time + delay_factor_ * ctrl_delay_, ctrl_send, &tap_receive );
	}

	Simulator::Destroy();

}

// This line creates a working FMU backend
CREATE_NS3_FMU_BACKEND( TC3CommNetworkFMU )
