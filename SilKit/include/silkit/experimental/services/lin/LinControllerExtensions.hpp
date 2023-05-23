/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "silkit/experimental/services/lin/LinDatatypesExtensions.hpp"
#include "silkit/services/lin/ILinController.hpp"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Services {
namespace Lin {

/*! \brief Add a LinSlaveConfigurationHandler on a given controller that triggers when a remote LIN slave is changes its configuration.
 *
 * This callback is mainly for diagnostic purposes and is NOT needed for regular LIN controller operation. 
 * It can be used to call \ref GetSlaveConfiguration to keep track of LIN Ids, where
 * a response of a LIN Slave is to be expected.
 * 
 * Requires \ref Services::Lin::LinControllerMode::Master.
 * 
 * \param linController The LIN controller to add the handler.
 * \param handler The callback that is triggered upon a configuration update.
 * 
 * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
 */
DETAIL_SILKIT_CPP_API auto AddLinSlaveConfigurationHandler(
    SilKit::Services::Lin::ILinController* linController,
    SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler) -> SilKit::Util::HandlerId;

/*! \brief Remove a LinSlaveConfigurationHandler by HandlerId on a given controller.
 *
 * Requires \ref Services::Lin::LinControllerMode::Master.
 * 
 * \param linController The LIN controller to remove the handler.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
DETAIL_SILKIT_CPP_API void RemoveLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* linController,
                                                              SilKit::Util::HandlerId handlerId);

/*! \brief Get the aggregated response configuration of all LIN slaves in the network.
 *
 * Requires \ref Services::Lin::LinControllerMode::Master.
 * 
 * \param linController The LIN controller (master) to providing the view.
 * 
 * \return A struct containing all LinIds on which LIN Slaves have configured Services::Lin::LinFrameResponseMode::TxUnconditional.
 */
DETAIL_SILKIT_CPP_API auto GetSlaveConfiguration(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration;

/*! \brief Initialize the LIN controller with the given LinControllerDynamicConfig
 *
 * All controllers must be initialized exactly once to take part in LIN communication.
 *
 * \param linController The controller to act upon
 * \param dynamicConfig The controller configuration contains:
 *  - controllerMode, either sets LIN master or LIN slave mode.
 *  - baudRate, determine transmission speeds (only used for detailed simulation).
 *  - simulationMode, can be used to enable LinSimulationMode::DynamicResponse
 *
 * \throws SilKit::StateError if the LIN Controller is configured with LinControllerMode::Inactive
 * \throws SilKit::StateError if Init() is called a second time on this LIN Controller.
 */
DETAIL_SILKIT_CPP_API void InitDynamic(SilKit::Services::Lin::ILinController* linController, const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig);

/*! \brief The FrameHeaderHandler is called whenever a LIN frame header is received.
 *
 * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
 */
DETAIL_SILKIT_CPP_API auto AddFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler) -> SilKit::Services::HandlerId;

/*! \brief Remove a FrameHeaderHandler by \ref SilKit::Util::HandlerId on this controller
 *
 * \param linController The controller to act upon
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
DETAIL_SILKIT_CPP_API void RemoveFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Services::HandlerId handlerId);

/*! \brief Send a response for the previously received LIN header, but only when the controller was initialized using \ref InitDynamic.
 *
 * \throws SilKit::StateError if the LIN Controller is not initialized.
 * \throws SilKit::StateError if the LIN controller was not initialized using \ref InitDynamic.
 * \throws SilKit::StateError if no prior LinFrameHeaderEvent was received before the call.
 */
DETAIL_SILKIT_CPP_API void SendDynamicResponse(SilKit::Services::Lin::ILinController* linController, const SilKit::Services::Lin::LinFrame& linFrame);

} // namespace Lin
} // namespace Services
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/experimental/services/lin/LinControllerExtensions.ipp"
//! \endcond
