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

#ifndef TC3_CONTROLLER_SERVER_H
#define TC3_CONTROLLER_SERVER_H

#include "server-base.h"


namespace ns3 {


/**
 * \ingroup tc3
 * \brief A TC3 controller server.
 */
class TC3ControllerServer : public ServerBase
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();

	TC3ControllerServer() {}

	virtual ~TC3ControllerServer() {}

	double GetEndToEndDelay_SMA() const { return smartMeterA_del; }
	double GetEndToEndDelay_SMB() const { return smartMeterB_del; }

protected:

	virtual void SetCallback();

private:

	/**
	 * \brief Handle a packet reception.
	 *
	 * This function is called by lower layers.
	 *
	 * \param socket the socket the packet was received to.
	 */
	void HandleRead( Ptr<Socket> socket );

	double smartMeterA_del;
	double smartMeterB_del;
	Address m_smartMeterA;
	Address m_smartMeterB;

};


} // namespace ns3


#endif // TC3_CONTROLLER_SERVER_H

