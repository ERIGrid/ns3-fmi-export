/* -*- Mode:C++; c-file-style:"bsd"; -*- */
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

#include "server-base.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "ServerBase" );

NS_OBJECT_ENSURE_REGISTERED( ServerBase );


ServerBase::ServerBase()
{
	NS_LOG_FUNCTION( this );
}


ServerBase::~ServerBase()
{
	NS_LOG_FUNCTION( this );
	m_socket = 0;
	m_socket6 = 0;
}


void
ServerBase::DoDispose()
{
	NS_LOG_FUNCTION( this );
	Application::DoDispose();
}


void
ServerBase::StartApplication()
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

	SetCallback();
}


void
ServerBase::StopApplication()
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


} // Namespace ns3
