// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <functional>

#include "ib/IbMacros.hpp"
#include "ib/vendor/IIbRegistry.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"


namespace ib {
namespace vendor {
inline namespace vector {

/*! \brief Create an instance of IIbRegistry.
*
* This API is specific to the Vector Informatik implementation of the IntegrationBus.
* It is required as a central service for other VIB participants to register with.
* Throws std::runtime_error on error.
*/

IntegrationBusAPI auto CreateIbRegistry(std::shared_ptr<cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>;

}// namespace vector
}// namespace vendor
}// namespace ib
