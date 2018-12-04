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

#ifndef CLIENT_BASE_H
#define CLIENT_BASE_H

#include <string>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"


namespace ns3 {

class Socket;
class Packet;


/**
 * \ingroup fmu-example
 * \brief Base class for clients. Adapted from class ns3::UdpEchoClient.
 * 
 * Inherited classes must declare static member function GetTypeId().
 * Inherited classes may re-implement member function SetCallback().
 */
class ClientBase : public Application
{

public:

	ClientBase();

	virtual ~ClientBase();

	/**
	 * \brief set the remote address and port
	 * \param ip remote IP address
	 * \param port remote port
	 */
	void SetRemote( Address ip, uint16_t port );
	/**
	 * \brief set the remote address
	 * \param addr remote address
	 */
	void SetRemote( Address addr );

	/**
	 * \param dataSize The size of the echo data you want to sent.
	 */
	void SetDataSize( uint32_t dataSize );

	/**
	 * \returns The number of data bytes.
	 */
	uint32_t GetDataSize() const;

	/**
	 * \param fill The string to use as the actual echo data bytes.
	 */
	void SetFill( std::string fill );

	/**
	 * \param fill The byte to be repeated in constructing the packet data..
	 * \param dataSize The desired size of the resulting echo packet data.
	 */
	void SetFill( uint8_t fill, uint32_t dataSize );

	/**
	 * \param fill The fill pattern to use when constructing packets.
	 * \param fillSize The number of bytes in the provided fill pattern.
	 * \param dataSize The desired size of the final echo data.
	 */
	void SetFill( uint8_t *fill, uint32_t fillSize, uint32_t dataSize );

protected:

	virtual void DoDispose();

	virtual void SetCallback() {}

private:

	virtual void StartApplication();
	virtual void StopApplication();

	/**
	 * \brief Schedule the next packet transmission
	 * \param dt time interval between packets.
	 */
	void ScheduleTransmit( Time dt );

	/**
	 * \brief Send a packet
	 */
	void Send();

protected:

	uint32_t m_count; //!< Maximum number of packets the application will send
	Time m_interval; //!< Packet inter-send time
	uint32_t m_size; //!< Size of the sent packet

	uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
	uint8_t *m_data; //!< packet payload data

	uint32_t m_sent; //!< Counter for sent packets
	Ptr<Socket> m_socket; //!< Socket
	Address m_peerAddress; //!< Remote peer address
	uint16_t m_peerPort; //!< Remote peer port
	EventId m_sendEvent; //!< Event to send the next packet

	/// Callbacks for tracing the packet Tx events
	TracedCallback<Ptr<const Packet> > m_txTrace;

};


} // namespace ns3


#endif // CLIENT_BASE_H
