// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/participant/exception.hpp"

#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/detail/impl/config/ParticipantConfiguration.hpp"

#include <string>
#include <vector>

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Config {

auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    SilKit_ParticipantConfiguration* participantConfiguration{nullptr};

    const auto returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, text.c_str());
    Impl::ThrowOnError(returnCode);

    return std::make_shared<Impl::Config::ParticipantConfiguration>(participantConfiguration);
}

auto ParticipantConfigurationFromFile(const std::string& path)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    SilKit_ParticipantConfiguration* participantConfiguration{nullptr};

    const auto returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfiguration, path.c_str());
    Impl::ThrowOnError(returnCode);

    return std::make_shared<Impl::Config::ParticipantConfiguration>(participantConfiguration);
}

auto ParticipantConfigurationToJson(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config) -> std::string
{
    size_t size{};
    auto&& concreteConfig = std::dynamic_pointer_cast<Impl::Config::ParticipantConfiguration>(config);
    auto returnCode = SilKit_ParticipantConfiguration_ToJson(concreteConfig->Get(), nullptr, &size);
    Impl::ThrowOnError(returnCode);

    std::vector<char> buffer;
    if (size > 0)
    {
        buffer.resize(size);
        auto&& data = buffer.data();
        returnCode = SilKit_ParticipantConfiguration_ToJson(concreteConfig->Get(), &data, &size);
        Impl::ThrowOnError(returnCode);
    }
    return {buffer.data(), buffer.size()};
}


} // namespace Config
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Config {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Config::ParticipantConfigurationFromString;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Config::ParticipantConfigurationFromFile;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Config::ParticipantConfigurationToJson;
} // namespace Config
} // namespace SilKit
