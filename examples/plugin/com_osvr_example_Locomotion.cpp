/** @date 2015
    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/LocomotionInterfaceC.h>

// Generated JSON header file
#include "com_osvr_example_Locomotion_json.h"

// Library/third-party includes

// Standard includes
#include <iostream>

// Anonymous namespace to avoid symbol collision
namespace {

OSVR_MessageType locomotionMessage;

class LocomotionDevice {
  public:
    LocomotionDevice(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        osvrDeviceLocomotionConfigure(opts, &m_locomotion);

        /// Create the sync device token with the options
        m_dev.initSync(ctx, "Locomotion", opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(com_osvr_example_Locomotion_json);

        /// Register update callback
        m_dev.registerUpdateCallback(this);
    }

    OSVR_ReturnCode update() {

        OSVR_TimeValue times;
        osvrTimeValueGetNow(&times);

        OSVR_NaviVelocityState velo;
        OSVR_NaviPositionState posn;

        velo.data[0] = 1.5;
        velo.data[1] = 9.9;
        posn.data[0] = 3.3;
        posn.data[2] = 7.7;

        osvrDeviceLocomotionReportNaviVelocity(m_locomotion, velo, 0, &times);

        osvrDeviceLocomotionReportNaviPosition(m_locomotion, posn, 0, &times);

        return OSVR_RETURN_SUCCESS;
    }

  private:
    osvr::pluginkit::DeviceToken m_dev;
    OSVR_LocomotionDeviceInterface m_locomotion;
};

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {

        if (m_found) {
            return OSVR_RETURN_SUCCESS;
        }

        std::cout << "PLUGIN: Got a hardware detection request" << std::endl;

        /// we always detect device in sample plugin
        m_found = true;

        std::cout << "PLUGIN: We have detected Locomotion device! "
                  << std::endl;
        /// Create our device object
        osvr::pluginkit::registerObjectForDeletion(ctx,
                                                   new LocomotionDevice(ctx));

        return OSVR_RETURN_SUCCESS;
    }

  private:
    bool m_found;
};
} // namespace

OSVR_PLUGIN(com_osvr_example_Locomotion) {

    osvrDeviceRegisterMessageType(ctx, "LocomotionMessage", &locomotionMessage);

    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}