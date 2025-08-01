// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/lin/ILinController.hpp"

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

using namespace SilKit::Services::Lin;

/*! \brief A LIN frame response update event delivered in the \ref LinSlaveConfigurationHandler
*
* The event is received on a LIN Master when a LIN Slave is configured via LinController::Init().
* This event is mainly for diagnostic purposes and can be used to keep track of LIN Ids, where
* a response of a LIN Slave is to be expected by using \ref GetSlaveConfiguration() in the handler.
*
*/
struct LinSlaveConfigurationEvent
{
    std::chrono::nanoseconds timestamp; //!< Time of the event.
};

/*! Callback type to indicate that a LIN Slave configuration has been received.
 *
 * Triggered when a remote LIN Slave calls LinController::Init() or LinController::SetFrameResponse().
 *  Cf., \ref AddLinSlaveConfigurationHandler
 */
using LinSlaveConfigurationHandler = ILinController::CallbackT<LinSlaveConfigurationEvent>;

//! \brief The aggregated configuration of all LIN slaves in the network.
struct LinSlaveConfiguration
{
    std::vector<LinId>
        respondingLinIds; //!< A vector of LinIds on which any LIN Slave has configured LinFrameResponseMode::TxUnconditional
};

/*! Configuration data to initialize the LIN Controller in the given LinSimulationMode
 *  Cf.: \ref InitDynamic(ILinController*,const LinControllerDynamicConfig&);
 */
struct LinControllerDynamicConfig
{
    //! Configure as LIN master or LIN slave
    LinControllerMode controllerMode{LinControllerMode::Inactive};
    /*! The operational baud rate of the controller. Used in a detailed simulation.
     */
    LinBaudRate baudRate{0};
};

//! \brief A frame header event delivered in the \ref LinFrameHeaderHandler.
struct LinFrameHeaderEvent
{
    std::chrono::nanoseconds timestamp; //!< Time of the event.
    LinId id;
};

/*! Callback type to indicate that a frame header has been received.
 *  Cf., \ref AddFrameHeaderHandler(ILinController*,LinFrameHeaderHandler);
 */
using LinFrameHeaderHandler = ILinController::CallbackT<LinFrameHeaderEvent>;

} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
