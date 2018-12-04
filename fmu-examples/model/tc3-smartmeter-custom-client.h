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

#ifndef TC3_SMARTMETER_CUSTOM_CLIENT_H
#define TC3_SMARTMETER_CUSTOM_CLIENT_H

#include "client-base.h"


namespace ns3 {

/**
 * \ingroup tc3
 * \brief A smart meter custom client.
 *
 * Every packet sent should be returned by the server and received here.
 */
class TC3SmartmeterCustomClient : public ClientBase
{

public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId();

	TC3SmartmeterCustomClient() {}

	virtual ~TC3SmartmeterCustomClient() {}

};


} // namespace ns3


#endif // TC3_SMARTMETER_CUSTOM_CLIENT_H
