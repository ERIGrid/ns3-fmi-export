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

#ifndef LSS2_CONTROLLER_SERVER_H
#define LSS2_CONTROLLER_SERVER_H

#include "server-base.h"


namespace ns3 {


// Create the struct/map here. It keeps delay + smartmeter address.
struct DelayInfo {
	std::string clientAddress;
	double endToEndDelay;
};


/**
 * \ingroup lss2
 * \brief A LSS2 controller server.
 */
class LSS2ControllerServer : public ServerBase
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();

	LSS2ControllerServer() : m_received( 0 ), m_lossCounter( 0 ) {}

	virtual ~LSS2ControllerServer() {}

	std::vector<DelayInfo> GetDeviceDelays() const;
	uint64_t GetReceived() const;
	uint32_t GetLost() const;

	uint16_t GetPacketWindowSize() const;
	void SetPacketWindowSize( uint16_t size );

protected:

	virtual void SetCallback();

private:

	void HandleRead( Ptr<Socket> socket );

	std::vector<DelayInfo> m_deviceInfo;
	uint64_t m_received;
	PacketLossCounter m_lossCounter;
};


} // namespace ns3


#endif // LSS2_CONTROLLER_SERVER_H
