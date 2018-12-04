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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/seq-ts-header.h"

#include "ns3/client-base.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "ClientBase" );

NS_OBJECT_ENSURE_REGISTERED( ClientBase );


ClientBase::ClientBase()
{
	NS_LOG_FUNCTION( this );
	m_sent = 0;
	m_socket = 0;
	m_sendEvent = EventId();
	m_data = 0;
	m_dataSize = 0;
}


ClientBase::~ClientBase()
{
	NS_LOG_FUNCTION( this );
	m_socket = 0;

	delete [] m_data;
	m_data = 0;
	m_dataSize = 0;
}


void
ClientBase::SetRemote( Address ip, uint16_t port )
{
	NS_LOG_FUNCTION( this << ip << port );
	m_peerAddress = ip;
	m_peerPort = port;
}


void
ClientBase::SetRemote( Address addr )
{
	NS_LOG_FUNCTION( this << addr );
	m_peerAddress = addr;
}


void
ClientBase::DoDispose()
{
	NS_LOG_FUNCTION( this );
	Application::DoDispose();
}


void
ClientBase::StartApplication()
{
	NS_LOG_FUNCTION( this );

	if ( m_socket == 0 )
	{
		TypeId tid = TypeId::LookupByName( "ns3::UdpSocketFactory" );
		m_socket = Socket::CreateSocket( GetNode(), tid );
		if ( Ipv4Address::IsMatchingType( m_peerAddress ) == true )
		{
			if ( m_socket->Bind() == -1 )
			{
				NS_FATAL_ERROR( "Failed to bind socket" );
			}
			m_socket->Connect( InetSocketAddress( Ipv4Address::ConvertFrom( m_peerAddress ), m_peerPort ) );
		}
		else if ( Ipv6Address::IsMatchingType( m_peerAddress ) == true )
		{
			if ( m_socket->Bind6() == -1 )
			{
				NS_FATAL_ERROR( "Failed to bind socket" );
			}
			m_socket->Connect( Inet6SocketAddress( Ipv6Address::ConvertFrom( m_peerAddress ), m_peerPort ) );
		}
		else if ( InetSocketAddress::IsMatchingType( m_peerAddress ) == true )
		{
			if ( m_socket->Bind() == -1 )
			{
				NS_FATAL_ERROR( "Failed to bind socket" );
			}
			m_socket->Connect( m_peerAddress );
		}
		else if ( Inet6SocketAddress::IsMatchingType( m_peerAddress ) == true )
		{
			if ( m_socket->Bind6() == -1 )
			{
				NS_FATAL_ERROR( "Failed to bind socket" );
			}
			m_socket->Connect( m_peerAddress );
		}
		else
		{
			NS_ASSERT_MSG( false, "Incompatible address type: " << m_peerAddress );
		}
	}

	SetCallback();
	m_socket->SetAllowBroadcast( true );
	ScheduleTransmit( Seconds( 0. ) );
}


void
ClientBase::StopApplication()
{
	NS_LOG_FUNCTION( this );

	if ( m_socket != 0 )
	{
		m_socket->Close();
		m_socket->SetRecvCallback( MakeNullCallback<void, Ptr<Socket> >() );
		m_socket = 0;
	}

	Simulator::Cancel( m_sendEvent );
}


void
ClientBase::SetDataSize( uint32_t dataSize )
{
	NS_LOG_FUNCTION( this << dataSize );

	//
	// If the client is setting the echo packet data size this way, we infer
	// that she doesn't care about the contents of the packet at all, so
	// neither will we.
	//
	delete [] m_data;
	m_data = 0;
	m_dataSize = 0;
	m_size = dataSize;
}


uint32_t
ClientBase::GetDataSize() const
{
	NS_LOG_FUNCTION( this );
	return m_size;
}


void
ClientBase::SetFill( std::string fill )
{
	NS_LOG_FUNCTION( this << fill );

	uint32_t dataSize = fill.size() + 1;

	if ( dataSize != m_dataSize )
	{
		delete [] m_data;
		m_data = new uint8_t [dataSize];
		m_dataSize = dataSize;
	}

	memcpy( m_data, fill.c_str(), dataSize );

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}


void
ClientBase::SetFill( uint8_t fill, uint32_t dataSize )
{
	NS_LOG_FUNCTION( this << fill << dataSize );
	if ( dataSize != m_dataSize )
	{
		delete [] m_data;
		m_data = new uint8_t [dataSize];
		m_dataSize = dataSize;
	}

	memset( m_data, fill, dataSize );

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}


void
ClientBase::SetFill( uint8_t *fill, uint32_t fillSize, uint32_t dataSize )
{
	NS_LOG_FUNCTION( this << fill << fillSize << dataSize );
	if ( dataSize != m_dataSize )
	{
		delete [] m_data;
		m_data = new uint8_t [dataSize];
		m_dataSize = dataSize;
	}

	if ( fillSize >= dataSize )
	{
		memcpy( m_data, fill, dataSize );
		m_size = dataSize;
		return;
	}

	//
	// Do all but the final fill.
	//
	uint32_t filled = 0;
	while ( filled + fillSize < dataSize )
	{
		memcpy( &m_data[filled], fill, fillSize );
		filled += fillSize;
	}

	//
	// Last fill may be partial
	//
	memcpy( &m_data[filled], fill, dataSize - filled );

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}


void
ClientBase::ScheduleTransmit( Time dt )
{
	NS_LOG_FUNCTION( this << dt );
	m_sendEvent = Simulator::Schedule( dt, &ClientBase::Send, this );
}


void
ClientBase::Send()
{
	NS_LOG_FUNCTION( this );

	NS_ASSERT( m_sendEvent.IsExpired() );

	// Integrate a header with the send time on the packet.
	SeqTsHeader seqTs;
	seqTs.SetSeq( m_sent );
	Ptr<Packet> p;
	if ( m_dataSize )
	{
		//
		// If m_dataSize is non-zero, we have a data buffer of the same size that we
		// are expected to copy and send.  This state of affairs is created if one of
		// the Fill functions is called.  In this case, m_size must have been set
		// to agree with m_dataSize
		//
		NS_ASSERT_MSG( m_dataSize == m_size, "ClientBase::Send(): m_size and m_dataSize inconsistent" );
		NS_ASSERT_MSG( m_data, "ClientBase::Send(): m_dataSize but no m_data" );
		p = Create<Packet>( m_data - ( 8+4 ), m_dataSize - ( 8+4 ) );

		// Integrate the seqTs to the packet.
		p->AddHeader( seqTs );
	}
	else
	{
		//
		// If m_dataSize is zero, the client has indicated that it doesn't care
		// about the data itself either by specifying the data size by setting
		// the corresponding attribute or by not calling a SetFill function.  In
		// this case, we don't worry about it either.  But we do allow m_size
		// to have a value different from the(zero) m_dataSize.
		//
		p = Create<Packet>( m_size - ( 8+4 ) );
	}

	// Integrate the seqTsHeader to the packet.
	p->AddHeader( seqTs );

	// call to the trace sinks before the packet is actually sent,
	// so that tags added to the packet can be sent as well
	m_txTrace( p );
	m_socket->Send( p );

	++m_sent;

	if ( Ipv4Address::IsMatchingType( m_peerAddress ) )
	{
		NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client sent " << m_size
			<< " bytes to " << Ipv4Address::ConvertFrom( m_peerAddress ) << " port " << m_peerPort );
	}
	else if ( Ipv6Address::IsMatchingType( m_peerAddress ) )
	{
		NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client sent " << m_size
			<< " bytes to " << Ipv6Address::ConvertFrom( m_peerAddress ) << " port " << m_peerPort );
	}
	else if ( InetSocketAddress::IsMatchingType( m_peerAddress ) )
	{
		NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client sent " << m_size
			<< " bytes to " << InetSocketAddress::ConvertFrom( m_peerAddress ).GetIpv4()
			<< " port " << InetSocketAddress::ConvertFrom( m_peerAddress ).GetPort() );
	}
	else if ( Inet6SocketAddress::IsMatchingType( m_peerAddress ) )
	{
		NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client sent " << m_size
			<< " bytes to " << Inet6SocketAddress::ConvertFrom( m_peerAddress ).GetIpv6()
			<< " port " << Inet6SocketAddress::ConvertFrom( m_peerAddress ).GetPort() );
	}

	if ( m_sent < m_count )
	{
		ScheduleTransmit( m_interval );
	}
}


} // Namespace ns3
