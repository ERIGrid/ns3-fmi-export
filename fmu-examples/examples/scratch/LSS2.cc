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

#include "ns3/lss2-controller-server.h"
#include "ns3/lss2-device-custom-client.h"
#include "ns3/lss2-helper.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <sstream>

// Include to test the MAX uint32 value
#include <stdint.h>

// Global variable to declare the maximum number of devices that the multiple input can support.
#define MAX_DEVICE_COUNT 100


using namespace ns3;

/*
* Geographically large scale system
* the scenario has the following topology:
*
* ex. 500 devices
* 10 seperate Wi-Fi networks that all connect to csma(ethernet) network where
* the cvc controller is in. All the devices send data at the same time to
* the cvc controller.
*
*  wifi1 (192.168.1.0/24)
*  *    *   ...  *    *   ---------------
* dev1 dev2     devN ap1                |               Ethernet/(10.0.0.1/24)
*                                       ----------------- Server/Controller
* wifi2 (192.168.2.0/24)                |
*  *     *  ...  *    *  ----------------
* dev1 dev2     devN ap2
*          ...
*          ...
*          ...
*/


NS_LOG_COMPONENT_DEFINE( "LSS2" );


class LSS2CommNetworkFMU : public SimpleEventQueueFMUBase
{

public:

	fmi2Integer interfere; // Input variable associated to whether or not we should simulate interference.

	fmiReal min_jitter; // Maximum jitter of sending devices.
	fmiReal max_jitter; // Minimum jitter of sending devices.

	fmi2Integer n_devices; // Number of devices used in the simulation.
	fmi2Integer divide_by; // Parameter to specify how many devices per WiFi should be instantiated.

	fmi2Integer devices_data_send[MAX_DEVICE_COUNT]; // Input variables associated to devices.
	fmi2Integer devices_data_receive[MAX_DEVICE_COUNT]; // Output variables associated to devices.

	fmi2Real max_device_delay; // Output variable for quantifying the network congestion (max. packet delay).

	fmi2Integer verbose; // Parameter for setting ns-3 verbosity.

	// Define the inputs outputs and parameters of the ns3 simulation.
	virtual void initializeSimulation();

	// Initialize the parameters of the simulation.
	virtual void initializeParameterValues();

	// Define the ns3 simulation that should be run.
	virtual void runSimulation( const double& sync_time );

private:

	std::unordered_map<std::string, int> device_number_map;

	fmi2Real jitter[MAX_DEVICE_COUNT]; // Fixed jitter for each device.

	void updateDelaysWithJitter( std::vector<DelayInfo> &delay_vector );

	int getIndex( DelayInfo delay_entry );

	std::string convert_to_string( Ipv4Address address );

	static void ArpCachTraceSink( Ptr<ns3::Packet const> packet );

	static bool sortOverload( DelayInfo d1, DelayInfo d2 );

};


void LSS2CommNetworkFMU::initializeSimulation()
{
	// Define parameter to turn verbosity on/off.
	addIntegerParameter( verbose );

	// Define parameter to turn interference on/off.
	addIntegerParameter( interfere );

	// Define parameter to specify how many devices per WiFi should be instantiated.
	addIntegerParameter( divide_by );

	// Define parameters to control amount of jitter of sending devices.
	addRealParameter( min_jitter )
	addRealParameter( max_jitter )

	// Define parameter to specify number of devices.
	addIntegerParameter( n_devices );

	// Define input/output variables associated to messages sent from the devices.
	for ( int i = 0; i < MAX_DEVICE_COUNT; ++i ) {
		addIntegerInputWithName( "device" + std::to_string(i) + "_data_send", devices_data_send[i] );
		addIntegerOutputWithName( "device" + std::to_string(i) + "_data_receive", devices_data_receive[i] );
	}

	// Define additional output variables with information about individual simulation runs.
	addRealOutput( max_device_delay );
}


void
LSS2CommNetworkFMU::initializeParameterValues()
{
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();

	for ( int i = 0; i < MAX_DEVICE_COUNT; ++i )
	jitter[i] = rand->GetValue( min_jitter, max_jitter );

	// Set default value.
	if ( 0 == divide_by ) divide_by = n_devices;
}


void LSS2CommNetworkFMU::runSimulation( const double& sync_time )
{
	// Check if any of the devices is sending.
	bool devices_sending = false;
	for ( int i = 0; i < n_devices; ++i ) { if ( devices_data_send[i] != 0 ) devices_sending = true; }

	// Cancel simulation run if devices are not sending.
	if ( false == devices_sending ) return;

	int numberOfWifis = n_devices / divide_by + 1;
	int remainingDevices = n_devices % divide_by;

	// std::cout << "n_devices = " << n_devices << std::endl;
	// std::cout << "numberOfWifis = " << numberOfWifis << std::endl;
	// std::cout << "remainingDevices = " << remainingDevices << std::endl;

	max_device_delay = 0;
	// double delays_sends = 0.001;

	std::string interval = "0.00001";
	uint32_t payloadSize = 972;

	/* Enable verbosity */
	if ( verbose ) {
		LogComponentEnable( "PacketLossCounter", LOG_LEVEL_INFO );
		LogComponentEnable( "LSS2", LOG_LEVEL_INFO );
		LogComponentEnable( "LSS2ControllerServer", LOG_LEVEL_INFO );
		LogComponentEnable( "LSS2DeviceCustomClient", LOG_LEVEL_INFO );
	}
	PacketMetadata::Enable();

	/* ------------- ns-3 components initialization ------------- */
	NodeContainer csmaNodes;
	csmaNodes.Create( numberOfWifis + 1 );

	NodeContainer wifiSta[numberOfWifis], wifiAP[numberOfWifis];
	NodeContainer wifiInterAP[numberOfWifis], wifiInterSta[numberOfWifis];

	// Create station nodes, devices and dummy devices for each WiFi.
	for ( int i = 0; i < numberOfWifis-1; ++i ) {
		wifiSta[i].Create( divide_by );
		wifiInterSta[i].Create(1);
		wifiInterAP[i].Create(1);
	}

	// Create the remaining devices in the last wifi network.
	wifiSta[numberOfWifis-1].Create( remainingDevices );
	wifiInterSta[numberOfWifis-1].Create(1);
	wifiInterAP[numberOfWifis-1].Create(1);

	for ( int i = 1; i < numberOfWifis+1; ++i ) wifiAP[i-1] = csmaNodes.Get(i);

	/* ------------- Physical layer & Data link (OSI layer 1 & 2) configuration------------- */
	YansWifiPhyHelper phy[numberOfWifis];
	for ( int i = 0; i < numberOfWifis; ++i ) phy[i] = YansWifiPhyHelper::Default();

	YansWifiChannelHelper channel[numberOfWifis];
	for ( int i = 0; i < numberOfWifis; ++i ) {
		channel[i].AddPropagationLoss( "ns3::FriisPropagationLossModel", "Frequency", DoubleValue( 5.180e9 ) );
		channel[i].SetPropagationDelay( "ns3::ConstantSpeedPropagationDelayModel" );

		phy[i].SetChannel( channel[i].Create() );
		phy[i].Set( "Frequency", UintegerValue( 5180 ) );
		phy[i].Set( "ShortGuardEnabled", BooleanValue( false ) );
		phy[i].Set( "ChannelWidth", UintegerValue( 40 ) );
	}

	WifiHelper wifi[numberOfWifis];
	WifiHelper wifiInter[numberOfWifis];
	WifiMacHelper mac[numberOfWifis], macInter[numberOfWifis];

	StringValue DataRate = StringValue( "HtMcs3" );

	for ( int i = 0; i < numberOfWifis; ++i ) {
		wifi[i].SetStandard( WIFI_PHY_STANDARD_80211ac );
		wifi[i].SetRemoteStationManager( "ns3::ConstantRateWifiManager","DataMode",
			DataRate, "ControlMode", DataRate );
		wifiInter[i].SetStandard( WIFI_PHY_STANDARD_80211ac );
		wifiInter[i].SetRemoteStationManager( "ns3::ConstantRateWifiManager","DataMode",
			DataRate, "ControlMode", DataRate );
	}

	/*------------ Installation of network devices to the components ------------*/
	CsmaHelper csma;
	NetDeviceContainer wifiStaDevices[numberOfWifis];
	NetDeviceContainer wifiAPDevices[numberOfWifis];
	NetDeviceContainer wifiInterStaDevices[numberOfWifis];
	NetDeviceContainer wifiInterAPDevices[numberOfWifis];
	NetDeviceContainer csmaDevices;

	csma.SetChannelAttribute( "DataRate", StringValue( "100Mbps" ) );
	csma.SetChannelAttribute( "Delay", TimeValue( NanoSeconds( 6560 ) ) );
	csmaDevices = csma.Install( csmaNodes );

	// wifi installation
	for ( int i = 0; i < numberOfWifis; ++i ) {
		std::stringstream stream;
		stream << "wifiAP" << i;
		Ssid ssid = Ssid( stream.str() );

		mac[i].SetType( "ns3::ApWifiMac", "Ssid", SsidValue( ssid ) );
		wifiAPDevices[i] = wifi[i].Install( phy[i], mac[i], wifiAP[i] );

		mac[i].SetType( "ns3::StaWifiMac", "Ssid", SsidValue( ssid ) );
		wifiStaDevices[i] = wifi[i].Install( phy[i], mac[i], wifiSta[i] );
	}

	// dummy wifi installation : the dummy wifis get the same phy as the devices
	for ( int i = 0; i < numberOfWifis; ++i ) {
		std::stringstream stream;
		stream << "wifiInterAP" << i;
		Ssid ssid = Ssid( stream.str() );

		macInter[i].SetType( "ns3::ApWifiMac", "Ssid", SsidValue( ssid ) );
		wifiInterAPDevices[i] = wifiInter[i].Install( phy[i], macInter[i], wifiInterAP[i] );

		macInter[i].SetType( "ns3::StaWifiMac","Ssid", SsidValue( ssid ) );
		wifiInterStaDevices[i] = wifiInter[i].Install( phy[i], macInter[i], wifiInterSta[i] );
	}

	/* ------------- Mobility/topology configuration------------- */
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc;
	positionAlloc = CreateObject<ListPositionAllocator>();
	const double PI = 3.14159;
	double radius = 200 + ( n_devices / 4 );
	double center_x = 0;
	double center_y = 0;
	double center_z = 0;
	double step = 2*PI / numberOfWifis;
	double angle = 0;

	// Keep track of the points that we add the access points
	double points_x[numberOfWifis], points_y[numberOfWifis];

	positionAlloc->Add( Vector( center_x, center_y, center_z ) );
	for ( int i = 0; i < numberOfWifis; ++i ) {
		points_x[i] = center_x + radius*cos( angle );
		points_y[i] = center_y + radius*sin( angle );
		positionAlloc->Add( Vector( points_x[i], points_y[i], center_z ) );
		angle += step;
	}

	/* Add the position of the controller which is in the last position of the
	csma devices */
	radius = 50;
	for ( int i = 0; i < numberOfWifis; ++i ) {
		step = 2*PI / wifiSta[i].GetN();
		angle = 0;
		for ( unsigned j = 0; j < wifiSta[i].GetN(); ++j ) {
			positionAlloc->Add(
				Vector( points_x[i] + radius*cos( angle ),
					points_y[i] + radius*sin( angle ),
					center_z ) );
			angle += step;
		}
	}

	// Mobility of the co-channel interfering wifi
	for ( int i = 0; i < numberOfWifis; ++i ) positionAlloc->Add( Vector( points_x[i] + 70, points_y[i], center_z ) );
	for ( int i = 0; i < numberOfWifis; ++i ) positionAlloc->Add( Vector( points_x[i] + 70, points_y[i] + 10, center_z ) );

	mobility.SetPositionAllocator( positionAlloc );
	mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
	mobility.Install( csmaNodes );

	for ( int i = 0; i < numberOfWifis; ++i ) mobility.Install( wifiSta[i] );
	for ( int i = 0; i < numberOfWifis; ++i ) mobility.Install( wifiInterSta[i] );
	for ( int i = 0; i < numberOfWifis; ++i ) mobility.Install( wifiInterAP[i] );


	/* ------------- Network layer (OSI layer 3) configuration------------- */
	InternetStackHelper stack;
	InternetStackHelper stack2;

	stack.Install( csmaNodes );
	for ( int i = 0; i < numberOfWifis; ++i ) stack.Install( wifiSta[i] );

	for ( int i = 0; i < numberOfWifis; ++i ) {
		stack2.Install( wifiInterSta[i] );
		stack2.Install( wifiInterAP[i] );
	}

	Ipv4AddressHelper address;
	Ipv4InterfaceContainer csmaInterfaces;
	Ipv4InterfaceContainer wifiInterfacesSta[numberOfWifis];
	Ipv4InterfaceContainer wifiInterfacesAP[numberOfWifis];
	Ipv4InterfaceContainer wifiInterStaInterfaces[numberOfWifis];
	Ipv4InterfaceContainer wifiInterAPInterfaces[numberOfWifis];

	// Assign IPs to csma network
	Ipv4Address csmaNetworkAddress( "10.1.1.0" );
	address.SetBase( csmaNetworkAddress, "255.255.255.0" );
	csmaInterfaces = address.Assign( csmaDevices );

	// Assign IPs to the device networks
	Ipv4Address wifiNetworkAddress[numberOfWifis];
	for ( int i = 0; i < numberOfWifis; ++i ) {
		std::stringstream ipv4Base;
		ipv4Base << "192.168." << i << ".0";
		const std::string temp = ipv4Base.str();
		const char *add = temp.c_str();
		wifiNetworkAddress[i].Set( add ); // needed for static routing
		address.SetBase( add, "255.255.255.0" );
		wifiInterfacesAP[i]  = address.Assign( wifiAPDevices[i] );
		wifiInterfacesSta[i] = address.Assign( wifiStaDevices[i] );
	}

	// Static routing
	Ptr<Ipv4> ipv4Controller = csmaNodes.Get(0)->GetObject<Ipv4>();
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> staticRoutingController =
	ipv4RoutingHelper.GetStaticRouting( ipv4Controller );

	for ( int i = 0; i < numberOfWifis; ++i ) {
		// Controller
		staticRoutingController->AddNetworkRouteTo( wifiNetworkAddress[i],
		Ipv4Mask( "255.255.255.0" ),
		csmaInterfaces.GetAddress( i+1 ), 1 );

		// Devices
		for ( unsigned j = 0; j < wifiSta[i].GetN(); ++j ) {
			Ptr<Ipv4> ipv4sm = wifiSta[i].Get(j)->GetObject<Ipv4>();
			Ptr<Ipv4StaticRouting> staticRoutingsm = ipv4RoutingHelper.GetStaticRouting( ipv4sm );
			staticRoutingsm->AddNetworkRouteTo( csmaNetworkAddress, Ipv4Mask( "255.255.255.0" ),
			wifiInterfacesAP[i].GetAddress(0), 1 );
		}
	}

	//Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	// Assign IPs to the dummy networks
	for ( int i = 0; i < numberOfWifis; ++i ) {
		std::stringstream ipv4Base;
		ipv4Base << "192.167." << i << ".0";
		const std::string temp = ipv4Base.str();
		const char *add = temp.c_str();
		address.SetBase( add, "255.255.255.0" );
		wifiInterAPInterfaces[i] = address.Assign( wifiInterAPDevices[i] );
		wifiInterStaInterfaces[i] = address.Assign( wifiInterStaDevices[i] );
	}

	/******** MESSAGE ID UPDATE **********/
	/* Initialize the device_number_map unordered_map if its not already initialized */
	if ( device_number_map.empty() ) {
		for ( int i = 0; i < numberOfWifis; ++i ) {
			for ( unsigned j = 0; j < wifiSta[i].GetN(); ++j ) {
				device_number_map[convert_to_string( wifiInterfacesSta[i].GetAddress(j) )] =
				i*divide_by + j;
			}
		}
	}

	// DEBUG: print the map to check if everything is OK
	//std::unordered_map<std::string, int>::iterator it = device_number_map.begin();
	//for ( ;it != device_number_map.end(); it++) {
	//    std::cout << it->first << " : " << it->second << std::endl;
	//}

	/* ------------- Application layer (OSI layer 7) configuration -------------*/
	LSS2DeviceCustomClientHelper deviceHlp( csmaInterfaces.GetAddress(0), 9 );
	deviceHlp.SetAttribute( "MaxPackets", UintegerValue(1) );
	deviceHlp.SetAttribute( "PacketSize", UintegerValue(100) );

	double startTime = 1.0;
	int current = 0;
	for ( int i = 0; i < numberOfWifis; ++i ) {
		//std::cout << "wifiSta[" << i << "].GetN() = " << wifiSta[i].GetN() << std::endl;
		for ( unsigned j = 0; j < wifiSta[i].GetN(); ++j ) {
			// if the device should send
			if ( devices_data_send[current++] != 0 ) {
				ApplicationContainer deviceApp;
				deviceApp = deviceHlp.Install( wifiSta[i].Get(j) );
				deviceApp.Start( Seconds( startTime + jitter[current-1] ) );
				deviceApp.Stop( Seconds( startTime + jitter[current-1] + 1.0 ) );
			}
		}

		// Install dummy devices if needed
		if ( interfere ) {
			Ipv4Address cnct_to = wifiInterAPInterfaces[i].GetAddress(0);
			LSS2DummyDeviceCustomClientHelper dummyDeviceHlp( cnct_to, 8 );
			dummyDeviceHlp.SetAttribute( "MaxPackets", UintegerValue( 4294967295u ) );
			dummyDeviceHlp.SetAttribute( "Interval", TimeValue( Time( interval ) ) );
			dummyDeviceHlp.SetAttribute( "PacketSize", UintegerValue( payloadSize ) );

			ApplicationContainer dummyClientrHlp =
			dummyDeviceHlp.Install( wifiInterSta[i].Get(0) );

			dummyClientrHlp.Start( Seconds( startTime - 0.5 ) );
			dummyClientrHlp.Stop( Seconds( startTime + 4.0 ) );

			//Install dummy device server
			LSS2DummyDeviceCustomServerHelper dummyDeviceServerHlp(8);

			ApplicationContainer dummyServer =
			dummyDeviceServerHlp.Install( wifiInterAP[i].Get(0) );

			dummyServer.Start( Seconds( startTime - 0.7 ) );
			dummyServer.Stop( Seconds( startTime + 4.0 ) );
		}
	}



	double stopTime = startTime + 1.0 + 3.5;

	// Controller application configuration
	LSS2ControllerServerHelper controllerHlp(9);
	ApplicationContainer controllerApp = controllerHlp.Install( csmaNodes.Get(0) );
	controllerApp.Start( Seconds( 0.0 ) );
	controllerApp.Stop( Seconds( stopTime ) );

	// Trace source if verbose mode on.
	if ( verbose )
	Config::ConnectWithoutContext( "/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/Drop",
		MakeCallback( &ArpCachTraceSink ) );

	Simulator::Stop( Seconds( stopTime ) );
	Simulator::Run();

	std::vector<DelayInfo> ete_delays =
		DynamicCast<LSS2ControllerServer>( controllerApp.Get(0) )->GetDeviceDelays();

	// Add additional jitter to the delays.
	updateDelaysWithJitter( ete_delays );

	//Sorting the delays to add them to the Event queue
	sort( ete_delays.begin(), ete_delays.end(), sortOverload );

	for ( DelayInfo info : ete_delays ) {
		// Retrieve associated msg_id.
		int device_number = device_number_map[info.clientAddress];
		fmi2Integer msg_id = devices_data_send[device_number];

		// Add new event to message queue.
		addNewEventForMessage( sync_time + info.endToEndDelay,
			msg_id,
			&devices_data_receive[device_number] );
	}

	// Get the maximum delay for the devices.
	max_device_delay = ete_delays.back().endToEndDelay;
	//std::cout << "The max delay is " << max_device_delay << std::endl;

	Simulator::Destroy();
}


/* Function which updates the delays with adding the jitter */
void
LSS2CommNetworkFMU::updateDelaysWithJitter( std::vector<DelayInfo>& delay_vector )
{
	std::vector<DelayInfo>::iterator start = delay_vector.begin();
	std::vector<DelayInfo>::iterator end = delay_vector.end();

	for ( auto it = start; it != end; ++it ) {
		it->endToEndDelay += jitter[getIndex( *it )];
	}
}


/* Extract the jitter index from the IP address. */
int
LSS2CommNetworkFMU::getIndex( DelayInfo delay_entry )
{
	std::size_t found = ( delay_entry.clientAddress ).find_last_of( "." );
	std::string number = ( delay_entry.clientAddress ).substr( found + 1 );
	return stoi( number, nullptr, 10 ) - 2;
}


/* Convert an ipv4 address to a string */
std::string
LSS2CommNetworkFMU::convert_to_string( Ipv4Address address ) {
	std::stringstream stream;
	address.Print( stream );
	return stream.str();
}


/* Callback function for when a packet is dropped in arp cache */
void
LSS2CommNetworkFMU::ArpCachTraceSink( Ptr<ns3::Packet const> packet )
{
	std::cout << "\nDropped with uid = " << packet->GetUid() << std::endl;
	packet->EnablePrinting();
	packet->Print( std::cout );
	packet->PrintByteTags( std::cout );
}


/* Function used to define the sorting pattern for the ete delays */
bool
LSS2CommNetworkFMU::sortOverload( DelayInfo d1, DelayInfo d2 )
{
	return d1.endToEndDelay < d2.endToEndDelay;
}


CREATE_NS3_FMU_BACKEND( LSS2CommNetworkFMU )
