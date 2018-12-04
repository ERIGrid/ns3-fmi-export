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

#ifndef LSS2_DUMMY_DEVICE_CUSTOM_SERVER_H
#define LSS2_DUMMY_DEVICE_CUSTOM_SERVER_H

#include "server-base.h"


namespace ns3 {


struct DummyDelayInfo {
	std::string clientAddress;
	double endToEndDelay;
};


/**
 * \ingroup tc3
 * \brief A dummy device custom server
 */
class LSS2DummyDeviceCustomServer : public ServerBase
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();

	LSS2DummyDeviceCustomServer() {}

	virtual ~LSS2DummyDeviceCustomServer() {}

	DummyDelayInfo GetDeviceDelay() { return controllerInfo; }

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

	DummyDelayInfo controllerInfo;
};


} // namespace ns3


#endif // LSS2_DUMMY_DEVICE_CUSTOM_SERVER_H
