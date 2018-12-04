/* -*- Mode:C++; c-file-style:"bsd"; -*- *//*
* Copyright 2007 University of Washington
* 
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

#ifndef TC3_CUSTOM_SERVER_H
#define TC3_CUSTOM_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"


namespace ns3 {


class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup tc3custom TC3Custom
 */

/**
 * \ingroup tc3custom
 * \brief A custom server for TC3.
 *
 * Every packet received is sent back.
 */
class TC3CustomServer : public Application 
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();
	TC3CustomServer();
	virtual ~TC3CustomServer();

	double GetEndToEndDelay() const { return ete_delay_; }

protected:

	virtual void DoDispose();

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

	uint16_t m_port; //!< Port on which we listen for incoming packets.
	Ptr<Socket> m_socket; //!< IPv4 Socket
	Ptr<Socket> m_socket6; //!< IPv6 Socket
	Address m_local; //!< local multicast address

	double ete_delay_;
};


} // namespace ns3


#endif /* TC3_CUSTOM_SERVER_H */
