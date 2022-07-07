// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <functional>

#include "silkit/SilKitMacros.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"


namespace SilKit {
namespace Vendor {
inline namespace Vector {

/*! \brief Create an instance of ISilKitRegistry.
*
* This API is specific to the Vector Informatik implementation of the SilKit.
* It is required as a central service for other SILKIT participants to register with.
* Throws std::runtime_error on error.
*/

SilKitAPI auto CreateSilKitRegistry(std::shared_ptr<Config::IParticipantConfiguration> config)
    -> std::unique_ptr<ISilKitRegistry>;

}// namespace Vector
}// namespace Vendor
}// namespace SilKit
