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

#include "tc3-controller-server.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "TC3ControllerServer" );

NS_OBJECT_ENSURE_REGISTERED( TC3ControllerServer );


TypeId
TC3ControllerServer::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::TC3ControllerServer" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<TC3ControllerServer>()
	.AddAttribute( "Port", "Port on which we listen for incoming packets.",
		UintegerValue( 9 ),
		MakeUintegerAccessor( &TC3ControllerServer::m_port ),
		MakeUintegerChecker<uint16_t>() )
	.AddAttribute( "SmartMeterA_Ipv4Address", "The address of SmartMeterA",
		AddressValue(),
		MakeAddressAccessor( &TC3ControllerServer::m_smartMeterA ),
		MakeAddressChecker() )
	.AddAttribute( "SmartMeterB_Ipv4Address", "The address of SmartMeterB",
		AddressValue(),
		MakeAddressAccessor( &TC3ControllerServer::m_smartMeterB ),
		MakeAddressChecker() );

	return tid;
}


void
TC3ControllerServer::SetCallback()
{
	m_socket->SetRecvCallback( MakeCallback( &TC3ControllerServer::HandleRead, this ) );
	m_socket6->SetRecvCallback( MakeCallback( &TC3ControllerServer::HandleRead, this ) );
}


void
TC3ControllerServer::HandleRead( Ptr<Socket> socket )
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

			if ( ipAddress == m_smartMeterA )
			{
				smartMeterA_del = Simulator::Now().GetSeconds() - seqTs.GetTs().GetSeconds();
				NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds()
					<< "s controller received " << packet->GetSize()
					<< " bytes from smartmeter (" << InetSocketAddress::ConvertFrom( from ).GetIpv4()
					<< ") port " << InetSocketAddress::ConvertFrom( from ).GetPort() );
			}
			else if ( ipAddress == m_smartMeterB )
			{
				smartMeterB_del = Simulator::Now().GetSeconds() - seqTs.GetTs().GetSeconds();
				NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds()
					<< "s controller received " << packet->GetSize()
					<< " bytes from smartmeterB (" << InetSocketAddress::ConvertFrom( from ).GetIpv4()
					<< ") port " << InetSocketAddress::ConvertFrom( from).GetPort() );
			}
		}
	}
}


} // Namespace ns3
