/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// --------------------------------------------------------------
// Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
// --------------------------------------------------------------

// ns-3 includes.
#include "ns3/core-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include "ns3/fmi-export-module.h"

#include "ns3/tc3-custom-server.h"
#include "ns3/tc3-custom-client.h"
#include "ns3/tc3-helper.h"

#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "SimpleFMU" );


class SimpleFMU : public SimpleEventQueueFMUBase {

public:

  // Define all FMI input/output variables and parameters as class members:
  fmi2Integer nodeA_send; // Input variable associated to nodeA.
  fmi2Integer nodeB_receive; // Output variable associated to nodeB.
  fmi2Real channel_delay; // Parameter for channel delay.
  
  // Define the inputs/outputs/parameters of the ns-3 simulation.
  virtual void initializeSimulation();

  // Define the ns-3 simulation that should be run.
  virtual void runSimulation( const double& sync_time );

};


void
SimpleFMU::initializeSimulation()
{
  // Define FMI integer input variable.
  addIntegerInput( nodeA_send );

  // Define FMI integer output variable.
  addIntegerOutput( nodeB_receive );

  // Define FMI parameter.
  addRealParameter( channel_delay );
}


void
SimpleFMU::runSimulation( const double& sync_time )
{
  // Check if nodeA received an input message. If not, don't run a simulation ...
  if ( 0 == nodeA_send ) return;

  std::stringstream str_delay;
  str_delay << channel_delay << "s";
  
  // Setup of the simulation.
  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper point_to_point;
  point_to_point.SetDeviceAttribute( "DataRate", StringValue( "5Mbps" ) );
  point_to_point.SetChannelAttribute( "Delay", StringValue( str_delay.str() ) );

  NetDeviceContainer devices;
  devices = point_to_point.Install( nodes );

  InternetStackHelper stack;
  stack.Install( nodes );

  Ipv4AddressHelper address;
  address.SetBase( "10.1.1.0", "255.255.255.0" );

  Ipv4InterfaceContainer interfaces = address.Assign( devices );

  uint16_t port = 9;
  
  TC3CustomServerHelper echo_server( port );

  ApplicationContainer server_apps = echo_server.Install( nodes.Get(1) );
  server_apps.Start( Seconds(1.0) );
  server_apps.Stop( Seconds(10.0) );

  TC3CustomClientHelper echo_client( interfaces.GetAddress(1), port );
  echo_client.SetAttribute( "MaxPackets", UintegerValue(1) );
  echo_client.SetAttribute( "Interval", TimeValue( Seconds(1.0) ) );
  echo_client.SetAttribute( "PacketSize", UintegerValue(1024) );

  ApplicationContainer client_apps = echo_client.Install( nodes.Get(0) );
  client_apps.Start( Seconds(2.0) );
  client_apps.Stop( Seconds(10.0) );

  // Run the simulation.
  Simulator::Run ();

  // Retrieve the massage delay.
  const TC3CustomServer& server = dynamic_cast< const TC3CustomServer& >( *server_apps.Get(0) ) ;
  double delay = server.GetEndToEndDelay();

  // Terminate the simulation.
  Simulator::Destroy ();

  // Add message as output to nodeB using the calculated delay.
  addNewEventForMessage( sync_time + delay, nodeA_send, &nodeB_receive );
  
}
  

// The next line creates a working FMU backend.
CREATE_NS3_FMU_BACKEND( SimpleFMU )
