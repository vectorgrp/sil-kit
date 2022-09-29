#pragma once

#include "silkit/SilKit.hpp"

namespace SilKit {
namespace Hourglass {

inline auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                              const std::string& participantName) -> std::unique_ptr<IParticipant>;

inline auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                              const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<IParticipant>;

} // namespace Hourglass
} // namespace SilKit

// ================================================================================
//  Inline Implementations
// ================================================================================

#include <memory>

#include "silkit/capi/SilKit.h"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/participant/Participant.hpp"
#include "silkit/hourglass/impl/config/ParticipantConfiguration.hpp"

namespace SilKit {
namespace Hourglass {

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName) -> std::unique_ptr<IParticipant>
{
    return CreateParticipant(std::move(participantConfig), participantName, std::string{});
}

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<IParticipant>
{
    auto& config = dynamic_cast<Impl::Config::ParticipantConfiguration&>(*participantConfig.get());

    SilKit_Participant* participant{nullptr};

    const auto returnCode =
        SilKit_Participant_Create(&participant, config.Get(), participantName.c_str(), registryUri.c_str());
    Impl::ThrowOnError(returnCode);

    return std::make_unique<Impl::Participant>(participant);
}

} // namespace Hourglass
} // namespace SilKit
