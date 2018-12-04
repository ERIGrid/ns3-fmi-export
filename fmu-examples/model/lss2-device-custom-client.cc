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

#include "ns3/log.h"
#include "ns3/uinteger.h"

#include "lss2-device-custom-client.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE( "LSS2DeviceCustomClient" );

NS_OBJECT_ENSURE_REGISTERED( LSS2DeviceCustomClient );


TypeId
LSS2DeviceCustomClient::GetTypeId()
{
	static TypeId tid = TypeId( "ns3::LSS2DeviceCustomClient" )
	.SetParent<Application>()
	.SetGroupName( "Applications" )
	.AddConstructor<LSS2DeviceCustomClient>()
	.AddAttribute( "MaxPackets",
		"The maximum number of packets the application will send",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &LSS2DeviceCustomClient::m_count ),
		MakeUintegerChecker<uint32_t>() )
	.AddAttribute( "Interval",
		"The time to wait between packets",
		TimeValue( Seconds( 1.0 ) ),
		MakeTimeAccessor( &LSS2DeviceCustomClient::m_interval ),
		MakeTimeChecker() )
	.AddAttribute( "RemoteAddress",
		"The destination Address of the outbound packets",
		AddressValue(),
		MakeAddressAccessor( &LSS2DeviceCustomClient::m_peerAddress ),
		MakeAddressChecker() )
	.AddAttribute( "RemotePort",
		"The destination port of the outbound packets",
		UintegerValue( 0 ),
		MakeUintegerAccessor( &LSS2DeviceCustomClient::m_peerPort ),
		MakeUintegerChecker<uint16_t>() )
	.AddAttribute( "PacketSize", "Size of echo data in outbound packets",
		UintegerValue( 100 ),
		MakeUintegerAccessor( &LSS2DeviceCustomClient::SetDataSize,
		&LSS2DeviceCustomClient::GetDataSize ),
		MakeUintegerChecker<uint32_t>() )
	.AddTraceSource( "Tx", "A new packet is created and is sent",
		MakeTraceSourceAccessor( &LSS2DeviceCustomClient::m_txTrace ),
		"ns3::Packet::TracedCallback" );

	return tid;
}


} // Namespace ns3
