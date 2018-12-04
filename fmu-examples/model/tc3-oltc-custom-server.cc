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

#include "tc3-oltc-custom-server.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE( "TC3OltcCustomServer" );

NS_OBJECT_ENSURE_REGISTERED( TC3OltcCustomServer );


using namespace ns3;


TypeId
TC3OltcCustomServer::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::TC3OltcCustomServer" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<TC3OltcCustomServer>()
	.AddAttribute( "Port", "Port on which we listen for incoming packets.",
		UintegerValue( 9 ),
		MakeUintegerAccessor( &TC3OltcCustomServer::m_port ),
		MakeUintegerChecker<uint16_t>() );

	return tid;
}


void
TC3OltcCustomServer::SetCallback()
{
	m_socket->SetRecvCallback( MakeCallback( &TC3OltcCustomServer::HandleRead, this ) );
	m_socket6->SetRecvCallback( MakeCallback( &TC3OltcCustomServer::HandleRead, this ) );
}


void
TC3OltcCustomServer::HandleRead( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION( this << socket );

	Ptr<Packet> packet;
	Address from;
	while ( ( packet = socket->RecvFrom( from ) ) )
	{
		SeqTsHeader seqTs;
		packet->PeekHeader( seqTs );
		controller_del = Simulator::Now().GetSeconds() - seqTs.GetTs().GetSeconds();

		if ( InetSocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds()
				<< "s OLTC received " << packet->GetSize()
				<< " bytes from the controller with IP: "<< InetSocketAddress::ConvertFrom( from ).GetIpv4()
				<< " port: " << InetSocketAddress::ConvertFrom( from ).GetPort() );
		}
		else if ( Inet6SocketAddress::IsMatchingType( from ) )
		{
			NS_LOG_INFO( "At time " << Simulator::Now().GetSeconds()
				<< "s OLTC received " << packet->GetSize()
				<< " bytes from the controller with IP: " << Inet6SocketAddress::ConvertFrom( from ).GetIpv6()
				<< " port: " << Inet6SocketAddress::ConvertFrom( from ).GetPort() );
		}

		packet->RemoveAllPacketTags();
		packet->RemoveAllByteTags();
	}

}


} // Namespace ns3
