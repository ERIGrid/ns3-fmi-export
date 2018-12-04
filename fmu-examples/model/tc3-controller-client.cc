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

#include "tc3-controller-client.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "TC3ControllerClient" );

NS_OBJECT_ENSURE_REGISTERED( TC3ControllerClient );


TypeId
TC3ControllerClient::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::TC3ControllerClient" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<TC3ControllerClient>()
	.AddAttribute( "MaxPackets",
		"The maximum number of packets the application will send",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &TC3ControllerClient::m_count ),
		MakeUintegerChecker<uint32_t>() )
	.AddAttribute( "Interval",
		"The time to wait between packets",
		TimeValue( Seconds( 1.0 ) ),
		MakeTimeAccessor( &TC3ControllerClient::m_interval ),
		MakeTimeChecker() )
	.AddAttribute( "RemoteAddress",
		"The destination Address of the outbound packets",
		AddressValue(),
		MakeAddressAccessor( &TC3ControllerClient::m_peerAddress ),
		MakeAddressChecker() )
	.AddAttribute( "RemotePort",
		"The destination port of the outbound packets",
		UintegerValue( 0 ),
		MakeUintegerAccessor( &TC3ControllerClient::m_peerPort ),
		MakeUintegerChecker<uint16_t>() )
	.AddAttribute( "PacketSize", "Size of echo data in outbound packets",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &TC3ControllerClient::SetDataSize,
		&TC3ControllerClient::GetDataSize ),
		MakeUintegerChecker<uint32_t>() )
	.AddTraceSource( "Tx", "A new packet is created and is sent",
		MakeTraceSourceAccessor( &TC3ControllerClient::m_txTrace ),
		"ns3::Packet::TracedCallback" );

	return tid;
}


} // Namespace ns3
