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

#ifndef LSS2_HELPER_H
#define LSS2_HELPER_H

#include "ns3/helper-base.icc"
#include "ns3/lss2-controller-server.h"
#include "ns3/lss2-device-custom-client.h"
#include "ns3/lss2-dummy-device-custom-client.h"
#include "ns3/lss2-dummy-device-custom-server.h"


namespace ns3 {

typedef ServerHelperBase<LSS2ControllerServer> LSS2ControllerServerHelper;

typedef ClientHelperBase<LSS2DeviceCustomClient> LSS2DeviceCustomClientHelper;

typedef ClientHelperBase<LSS2DummyDeviceCustomClient> LSS2DummyDeviceCustomClientHelper;

typedef ServerHelperBase<LSS2DummyDeviceCustomServer> LSS2DummyDeviceCustomServerHelper;

} // namespace ns3


#endif // LSS2_HELPER_H
