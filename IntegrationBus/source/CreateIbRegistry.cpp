// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>
#include <memory>

#include "ib/vendor/CreateIbRegistry.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "ParticipantConfiguration.hpp"
#include "VAsioRegistry.hpp"


namespace ib {
namespace vendor {
inline namespace vector {

auto CreateIbRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<vendor::IIbRegistry>
{
    return std::make_unique<mw::VAsioRegistry>(config);
}


} // namespace vector
} // namespace vendor
} // namespace ib
