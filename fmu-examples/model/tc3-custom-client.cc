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
#include "ns3/uinteger.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/seq-ts-header.h"

#include "tc3-custom-client.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "TC3CustomClient" );

NS_OBJECT_ENSURE_REGISTERED( TC3CustomClient );


TypeId
TC3CustomClient::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::TC3CustomClient" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<TC3CustomClient>()
	.AddAttribute( "MaxPackets",
		"The maximum number of packets the application will send",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &TC3CustomClient::m_count ),
		MakeUintegerChecker<uint32_t>() )
	.AddAttribute( "Interval",
		"The time to wait between packets",
		TimeValue( Seconds( 1.0 ) ),
		MakeTimeAccessor( &TC3CustomClient::m_interval ),
		MakeTimeChecker() )
	.AddAttribute( "RemoteAddress",
		"The destination Address of the outbound packets",
		AddressValue(),
		MakeAddressAccessor( &TC3CustomClient::m_peerAddress ),
		MakeAddressChecker() )
	.AddAttribute( "RemotePort",
		"The destination port of the outbound packets",
		UintegerValue( 0 ),
		MakeUintegerAccessor( &TC3CustomClient::m_peerPort ),
		MakeUintegerChecker<uint16_t>() )
	.AddAttribute( "PacketSize", "Size of echo data in outbound packets",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &TC3CustomClient::SetDataSize,
		&TC3CustomClient::GetDataSize ),
		MakeUintegerChecker<uint32_t>() )
	.AddTraceSource( "Tx", "A new packet is created and is sent",
		MakeTraceSourceAccessor( &TC3CustomClient::m_txTrace ),
		"ns3::Packet::TracedCallback" );

	return tid;
}


void
TC3CustomClient::HandleRead( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION( this << socket );
	Ptr<Packet> packet;
	Address from;
	while ( ( packet = socket->RecvFrom( from ) ) )
	{
		if ( InetSocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client received " << packet->GetSize() << " bytes from " <<
			InetSocketAddress::ConvertFrom( from ).GetIpv4() << " port " <<
			InetSocketAddress::ConvertFrom( from ).GetPort() );
		}
		else if ( Inet6SocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() << "s client received " << packet->GetSize() << " bytes from " <<
			Inet6SocketAddress::ConvertFrom( from ).GetIpv6() << " port " <<
			Inet6SocketAddress::ConvertFrom( from ).GetPort() );
		}
	}
}


void
TC3CustomClient::SetCallback()
{
	m_socket->SetRecvCallback( MakeCallback( &TC3CustomClient::HandleRead, this ) );
}


} // Namespace ns3
