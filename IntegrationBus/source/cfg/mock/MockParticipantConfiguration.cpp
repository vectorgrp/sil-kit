// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "MockParticipantConfiguration.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace cfg {

auto MockParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    return std::make_shared<ib::cfg::v1::datatypes::ParticipantConfiguration>();
}

} // namespace cfg
} // namespace ib
