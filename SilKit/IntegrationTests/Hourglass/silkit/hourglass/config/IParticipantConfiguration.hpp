#pragma once

#include "silkit/config/IParticipantConfiguration.hpp"

namespace SilKit {
namespace Hourglass {
namespace Config {

inline auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

inline auto ParticipantConfigurationFromFile(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

} // namespace Config
} // namespace Hourglass
} // namespace SilKit

// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/participant/exception.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/Macros.hpp"
#include "silkit/hourglass/impl/config/ParticipantConfiguration.hpp"

#include <sstream>
#include <fstream>

namespace SilKit {
namespace Hourglass {
namespace Config {

auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    SilKit_ParticipantConfiguration* participantConfiguration{nullptr};

    const auto returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, text.c_str());
    Impl::ThrowOnError(returnCode);

    return std::make_shared<Impl::Config::ParticipantConfiguration>(participantConfiguration);
}

auto ParticipantConfigurationFromFile(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    std::ifstream fs(filename);

    if (!fs.is_open())
        throw SilKit::ConfigurationError("the file could not be opened");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return ParticipantConfigurationFromString(buffer.str());
}

} // namespace Config
} // namespace Hourglass
} // namespace SilKit
