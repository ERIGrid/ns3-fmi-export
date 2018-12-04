/* -*- Mode:C++; c-file-style:"bsd"; -*- *//*
* Copyright 2007 University of Washington
* 
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include "tc3-custom-server.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "TC3CustomServer" );

NS_OBJECT_ENSURE_REGISTERED( TC3CustomServer );


TypeId
TC3CustomServer::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::TC3CustomServer" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<TC3CustomServer>()
	.AddAttribute( "Port", "Port on which we listen for incoming packets.",
		UintegerValue( 9 ),
		MakeUintegerAccessor( &TC3CustomServer::m_port ),
		MakeUintegerChecker<uint16_t>() );
	
	return tid;
}


TC3CustomServer::TC3CustomServer()
{
	NS_LOG_FUNCTION( this );
	ete_delay_ = -1;
}


TC3CustomServer::~TC3CustomServer( )
{
	NS_LOG_FUNCTION( this);
	m_socket = 0;
	m_socket6 = 0;
}


void
TC3CustomServer::DoDispose()
{
	NS_LOG_FUNCTION( this );
	Application::DoDispose();
}


void 
TC3CustomServer::StartApplication()
{
	NS_LOG_FUNCTION( this );

	if ( m_socket == 0 )
	{
		TypeId tid = TypeId::LookupByName( "ns3::UdpSocketFactory" );
		m_socket = Socket::CreateSocket( GetNode(), tid );
		InetSocketAddress local = InetSocketAddress( Ipv4Address::GetAny(), m_port );
		if ( m_socket->Bind( local ) == -1 )
		{
			NS_FATAL_ERROR( "Failed to bind socket" );
		}
		if ( addressUtils::IsMulticast( m_local ) )
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>( m_socket );
			if ( udpSocket )
			{
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup( 0, m_local );
			}
			else
			{
				NS_FATAL_ERROR( "Error: Failed to join multicast group" );
			}
		}
	}

	if ( m_socket6 == 0 )
	{
		TypeId tid = TypeId::LookupByName( "ns3::UdpSocketFactory" );
		m_socket6 = Socket::CreateSocket( GetNode(), tid );
		Inet6SocketAddress local6 = Inet6SocketAddress( Ipv6Address::GetAny(), m_port );
		if ( m_socket6->Bind( local6 ) == -1 )
		{
			NS_FATAL_ERROR( "Failed to bind socket" );
		}
		if ( addressUtils::IsMulticast( local6 ) )
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>( m_socket6 );
			if ( udpSocket )
			{
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup( 0, local6 );
			}
			else
			{
				NS_FATAL_ERROR( "Error: Failed to join multicast group" );
			}
		}
	}

	m_socket->SetRecvCallback( MakeCallback( &TC3CustomServer::HandleRead, this ) );
	m_socket6->SetRecvCallback( MakeCallback( &TC3CustomServer::HandleRead, this ) );
}


void 
TC3CustomServer::StopApplication()
{
	NS_LOG_FUNCTION( this );

	if ( m_socket != 0 ) 
	{
		m_socket->Close();
		m_socket->SetRecvCallback( MakeNullCallback<void, Ptr<Socket> >() );
	}
	if ( m_socket6 != 0 ) 
	{
		m_socket6->Close();
		m_socket6->SetRecvCallback( MakeNullCallback<void, Ptr<Socket> >() );
	}
}


void 
TC3CustomServer::HandleRead( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION( this << socket );

	Ptr<Packet> packet;
	Address from;
	//added by me: we need to retrieve the header Ts indicating the send time of the packet 
	while ( ( packet = socket->RecvFrom( from ) ) )
	{
		//create temp variables to store the header and calculate ete delay. 
		//the objects created will not be affected by the ns3 garbage collector
		// --> memory leaks! need to resolve
		SeqTsHeader st_header;
		packet->RemoveHeader( st_header );
		Time t_diff = Simulator::Now() - st_header.GetTs();

		ete_delay_ = Simulator::Now().GetSeconds() - st_header.GetTs().GetSeconds();
		//printf( "\nNs3: Inside the application layer. The value of ete delay is %f\n", ete_delay_ );

		if ( InetSocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s server received " << packet->GetSize() << " bytes from " <<
			InetSocketAddress::ConvertFrom( from ).GetIpv4() << " port " <<
			InetSocketAddress::ConvertFrom( from ).GetPort() );
		}
		else if ( Inet6SocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s server received " << packet->GetSize() << " bytes from " <<
			Inet6SocketAddress::ConvertFrom( from ).GetIpv6() << " port " <<
			Inet6SocketAddress::ConvertFrom( from ).GetPort() );
		}

		packet->RemoveAllPacketTags();
		packet->RemoveAllByteTags();

		NS_LOG_LOGIC( "Echoing packet" );
		socket->SendTo( packet, 0, from );

		if ( InetSocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s server sent " << packet->GetSize() << " bytes to " <<
			InetSocketAddress::ConvertFrom( from ).GetIpv4() << " port " <<
			InetSocketAddress::ConvertFrom( from ).GetPort() );
		}
		else if ( Inet6SocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s server sent " << packet->GetSize() << " bytes to " <<
			Inet6SocketAddress::ConvertFrom( from ).GetIpv6() << " port " <<
			Inet6SocketAddress::ConvertFrom( from ).GetPort() );
		}
	}
}


} // Namespace ns3
