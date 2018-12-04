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

#ifndef TC3_HELPER_H
#define TC3_HELPER_H

#include "ns3/helper-base.icc"
#include "ns3/tc3-controller-server.h"
#include "ns3/tc3-controller-client.h"
#include "ns3/tc3-custom-server.h"
#include "ns3/tc3-custom-client.h"
#include "ns3/tc3-oltc-custom-server.h"
#include "ns3/tc3-smartmeter-custom-client.h"


namespace ns3 {

typedef ServerHelperBase<TC3ControllerServer> TC3ControllerServerHelper;

typedef ClientHelperBase<TC3ControllerClient> TC3ControllerClientHelper;

typedef ServerHelperBase<TC3CustomServer> TC3CustomServerHelper;

typedef ClientHelperBase<TC3CustomClient> TC3CustomClientHelper;

typedef ServerHelperBase<TC3OltcCustomServer> TC3OltcCustomServerHelper;

typedef ClientHelperBase<TC3SmartmeterCustomClient> TC3SmartmeterCustomClientHelper;

} // namespace ns3


#endif // TC3_HELPER_H