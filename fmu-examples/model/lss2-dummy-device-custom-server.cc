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

#include "lss2-dummy-device-custom-server.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "LSS2DummyDeviceCustomServer" );

NS_OBJECT_ENSURE_REGISTERED( LSS2DummyDeviceCustomServer );

using namespace ns3;


TypeId
LSS2DummyDeviceCustomServer::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::LSS2DummyDeviceCustomServer" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<LSS2DummyDeviceCustomServer>()
	.AddAttribute( "Port", "Port on which we listen for incoming packets.",
		UintegerValue( 9 ),
		MakeUintegerAccessor( &LSS2DummyDeviceCustomServer::m_port ),
		MakeUintegerChecker<uint16_t>() );

	return tid;
}


void
LSS2DummyDeviceCustomServer::SetCallback()
{
	m_socket->SetRecvCallback( MakeCallback( &LSS2DummyDeviceCustomServer::HandleRead, this ) );
	m_socket6->SetRecvCallback( MakeCallback( &LSS2DummyDeviceCustomServer::HandleRead, this ) );
}


void
LSS2DummyDeviceCustomServer::HandleRead( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION( this << socket );

	Ptr<Packet> packet;
	Address from;
	while ( ( packet = socket->RecvFrom( from ) ) )
	{
		SeqTsHeader seqTs;
		packet->PeekHeader( seqTs );
		double controller_del = Simulator::Now().GetSeconds() - seqTs.GetTs().GetSeconds();

		Ipv4Address ipAddress = InetSocketAddress::ConvertFrom( from ).GetIpv4();
		std::stringstream IpAddrStrStrm;
		ipAddress.Print( IpAddrStrStrm );
		std::string IpAddrStr = IpAddrStrStrm.str();

		controllerInfo.clientAddress = IpAddrStr;
		controllerInfo.endToEndDelay = controller_del;

		if ( InetSocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() 
				<< "s dummy server received " << packet->GetSize() 
				<< " bytes from the dummy meter with IP: "<< InetSocketAddress::ConvertFrom( from ).GetIpv4()
				<< " port: " << InetSocketAddress::ConvertFrom( from ).GetPort() );
		}
		else if ( Inet6SocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds() 
				<< "s dummy server received " << packet->GetSize()
				<< " bytes from the dummy meter with IP: " << Inet6SocketAddress::ConvertFrom( from ).GetIpv6()
				<< " port: " << Inet6SocketAddress::ConvertFrom( from ).GetPort() );
		}

		packet->RemoveAllPacketTags();
		packet->RemoveAllByteTags();
	}
}


} // namespace ns3
