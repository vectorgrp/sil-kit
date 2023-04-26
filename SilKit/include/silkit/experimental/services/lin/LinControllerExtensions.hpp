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

} // namespace Lin
} // namespace Services
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/experimental/services/lin/LinControllerExtensions.ipp"
//! \endcond
