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

#ifndef SERVER_BASE_H
#define SERVER_BASE_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/packet-loss-counter.h"
#include "ns3/ipv4-address.h"


namespace ns3 {


class Socket;
class Packet;

/**
 * \ingroup fmu-example
 * \brief Base class for servers. Adapted from class ns3::UdpServer.
 * 
 * Inherited classes must declare static member function GetTypeId().
 * Inherited classes may re-implement member function SetCallback().
 */
class ServerBase : public Application
{

public:

	ServerBase();

	virtual ~ServerBase();

protected:

	virtual void DoDispose();

	virtual void SetCallback() {}
	
private:

	virtual void StartApplication();
	virtual void StopApplication();

	/**
	 * \brief Handle a packet reception.
	 *
	 * This function is called by lower layers.
	 *
	 * \param socket the socket the packet was received to.
	 */
	void HandleRead( Ptr<Socket> socket );

protected:
	
	uint16_t m_port; //!< Port on which we listen for incoming packets.
	Ptr<Socket> m_socket; //!< IPv4 Socket
	Ptr<Socket> m_socket6; //!< IPv6 Socket
	Address m_local; //!< local multicast address

	double smartMeterA_del;
	double smartMeterB_del;
	Address m_smartMeterA;
	Address m_smartMeterB;

};


} // namespace ns3


#endif // SERVER_BASE_H

