#pragma once

#ifdef SILKIT_HOURGLASS
#    include "silkit/hourglass/SilKit.hpp"
#    include "silkit/hourglass/config/IParticipantConfiguration.hpp"
#else
#    include "silkit/SilKit.hpp"
#    include "silkit/config/IParticipantConfiguration.hpp"
#endif

namespace SilKit {
namespace IntegrationTests {

#ifdef SILKIT_HOURGLASS

template <typename... Ts>
auto CreateParticipant(Ts &&...ts) -> std::unique_ptr<SilKit::IParticipant>
{
    return SilKit::Hourglass::CreateParticipant(std::forward<Ts>(ts)...);
}

template <typename... Ts>
auto ParticipantConfigurationFromString(Ts &&...ts) -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return SilKit::Hourglass::Config::ParticipantConfigurationFromString(std::forward<Ts>(ts)...);
}

#else

template <typename... Ts>
auto CreateParticipant(Ts &&...ts) -> std::unique_ptr<SilKit::IParticipant>
{
    return SilKit::CreateParticipant(std::forward<Ts>(ts)...);
}

template <typename... Ts>
auto ParticipantConfigurationFromString(Ts &&...ts) -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return SilKit::Config::ParticipantConfigurationFromString(std::forward<Ts>(ts)...);
}

#endif

} // namespace IntegrationTests
} // namespace SilKit
