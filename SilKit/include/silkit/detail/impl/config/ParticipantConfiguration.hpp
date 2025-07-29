// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/config/IParticipantConfiguration.hpp"

#include "silkit/capi/SilKit.h"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Config {

class ParticipantConfiguration : public SilKit::Config::IParticipantConfiguration
{
public:
    inline explicit ParticipantConfiguration(SilKit_ParticipantConfiguration* participantConfiguration);

    inline ~ParticipantConfiguration() override;

public:
    inline auto Get() const -> SilKit_ParticipantConfiguration*;

private:
    SilKit_ParticipantConfiguration* _participantConfiguration{nullptr};
};

} // namespace Config
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Config {

ParticipantConfiguration::ParticipantConfiguration(SilKit_ParticipantConfiguration* participantConfiguration)
    : _participantConfiguration{participantConfiguration}
{
}

ParticipantConfiguration::~ParticipantConfiguration()
{
    if (_participantConfiguration != nullptr)
    {
        SilKit_ParticipantConfiguration_Destroy(_participantConfiguration);
    }
}

auto ParticipantConfiguration::Get() const -> SilKit_ParticipantConfiguration*
{
    return _participantConfiguration;
}

} // namespace Config
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
