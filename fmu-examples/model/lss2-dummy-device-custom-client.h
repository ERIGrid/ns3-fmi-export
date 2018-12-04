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

#ifndef LSS2_DUMMY_DEVICE_CUSTOM_CLIENT_H
#define LSS2_DUMMY_DEVICE_CUSTOM_CLIENT_H

#include "client-base.h"


namespace ns3 {


/**
 * \ingroup lss2
 * \brief A dummy device custom client
 *
 * Every packet sent should be returned by the server and received here.
 */
class LSS2DummyDeviceCustomClient : public ClientBase
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();

	LSS2DummyDeviceCustomClient() {}

	virtual ~LSS2DummyDeviceCustomClient() {}

};


} // namespace ns3


#endif // LSS2_DUMMY_DEVICE_CUSTOM_CLIENT_H
