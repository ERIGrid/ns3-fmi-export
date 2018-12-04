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
#include "ns3/packet-loss-counter.h"

#include "lss2-controller-server.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "LSS2ControllerServer" );

NS_OBJECT_ENSURE_REGISTERED( LSS2ControllerServer );


TypeId
LSS2ControllerServer::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::LSS2ControllerServer" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<LSS2ControllerServer>()
	.AddAttribute( "Port", "Port on which we listen for incoming packets.",
		UintegerValue( 9 ),
		MakeUintegerAccessor( &LSS2ControllerServer::m_port ),
		MakeUintegerChecker<uint16_t>() )
	.AddAttribute( "PacketWindowSize",
		"The size of the window used to compute the packet loss. This value should be a multiple of 8.",
		UintegerValue( 32 ),
		MakeUintegerAccessor( &LSS2ControllerServer::GetPacketWindowSize,
		&LSS2ControllerServer::SetPacketWindowSize ),
		MakeUintegerChecker<uint16_t>( 8,256 ) );

	return tid;
}


uint16_t
LSS2ControllerServer::GetPacketWindowSize() const
{
	NS_LOG_FUNCTION( this );
	return m_lossCounter.GetBitMapSize();
}


void
LSS2ControllerServer::SetPacketWindowSize( uint16_t size )
{
	NS_LOG_FUNCTION( this << size );
	m_lossCounter.SetBitMapSize( size );
}


uint32_t
LSS2ControllerServer::GetLost() const
{
	NS_LOG_FUNCTION( this );
	return m_lossCounter.GetLost();
}


void
LSS2ControllerServer::SetCallback()
{
	m_socket->SetRecvCallback( MakeCallback( &LSS2ControllerServer::HandleRead, this ) );
	m_socket6->SetRecvCallback( MakeCallback( &LSS2ControllerServer::HandleRead, this ) );
}


void
LSS2ControllerServer::HandleRead( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION( this << socket );

	Ptr<Packet> packet;
	Address from;
	while ( ( packet = socket->RecvFrom( from ) ) )
	{
		if ( packet->GetSize() > 0 )
		{

			SeqTsHeader seqTs;
			packet->PeekHeader( seqTs,12 );
			Ipv4Address ipAddress = InetSocketAddress::ConvertFrom( from ).GetIpv4();
			uint32_t currentSequenceNumber = seqTs.GetSeq();

			std::stringstream IpAddrStrStrm;

			ipAddress.Print( IpAddrStrStrm );

			std::string IpAddrStr = IpAddrStrStrm.str();

			DelayInfo temp;
			temp.clientAddress = IpAddrStr;
			temp.endToEndDelay = ( Simulator::Now() - seqTs.GetTs() ).GetSeconds();

			// We have the sender IP address as a string. We can use it as the key in our map, or the name variable in the struct.
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds()
				<< "s controller received " << packet->GetSize()
				<< " bytes from smartmeterA (" << InetSocketAddress::ConvertFrom( from ).GetIpv4() 
				<< ") port " << InetSocketAddress::ConvertFrom( from ).GetPort() );

			m_deviceInfo.push_back( temp );
			m_lossCounter.NotifyReceived( currentSequenceNumber );
			m_received++;
		}


	}
}


std::vector<DelayInfo>
LSS2ControllerServer::GetDeviceDelays() const
{
	return m_deviceInfo;
}


uint64_t
LSS2ControllerServer::GetReceived() const
{
	NS_LOG_FUNCTION( this );
	return m_received;
}


} // Namespace ns3
