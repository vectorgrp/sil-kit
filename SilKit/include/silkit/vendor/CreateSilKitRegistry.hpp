// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include "silkit/SilKitMacros.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Vendor {
namespace Vector {

/*! \brief Create an instance of ISilKitRegistry.
 *
 * This API is specific to the Vector Informatik implementation of the SIL Kit.
 * It is required as a central service for other SIL Kit participants to register with.
 *
 * \throws SilKit::SilKitError on error.
 */
DETAIL_SILKIT_CPP_API auto CreateSilKitRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>;

} // namespace Vector
} // namespace Vendor
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/vendor/CreateSilKitRegistry.ipp"
//! \endcond
