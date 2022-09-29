#pragma once

#include "silkit/config/IParticipantConfiguration.hpp"

#include "silkit/capi/SilKit.h"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Config {

class ParticipantConfiguration : public SilKit::Config::IParticipantConfiguration
{
public:
    explicit ParticipantConfiguration(SilKit_ParticipantConfiguration* participantConfiguration)
        : _participantConfiguration{participantConfiguration}
    {
    }

    ~ParticipantConfiguration() override
    {
        if (_participantConfiguration != nullptr)
        {
            SilKit_ParticipantConfiguration_Destroy(_participantConfiguration);
        }
    }

    auto Get() const -> SilKit_ParticipantConfiguration*
    {
        return _participantConfiguration;
    }

private:
    SilKit_ParticipantConfiguration* _participantConfiguration{nullptr};
};

} // namespace Config
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
