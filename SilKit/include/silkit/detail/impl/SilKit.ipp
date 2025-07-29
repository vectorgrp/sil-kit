// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include "silkit/capi/SilKit.h"

#include "silkit/detail/impl/participant/Participant.hpp"
#include "silkit/detail/impl/config/ParticipantConfiguration.hpp"

#include "ThrowOnError.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName) -> std::unique_ptr<IParticipant>
{
    return CreateParticipant(std::move(participantConfig), participantName, std::string{});
}

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName,
                       const std::string& registryUri) -> std::unique_ptr<IParticipant>
{
    auto& config = dynamic_cast<Impl::Config::ParticipantConfiguration&>(*participantConfig.get());

    SilKit_Participant* participant{nullptr};

    const auto returnCode =
        SilKit_Participant_Create(&participant, config.Get(), participantName.c_str(), registryUri.c_str());
    Impl::ThrowOnError(returnCode);

    return std::make_unique<Impl::Participant>(participant);
}

DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::CreateParticipant;
} // namespace SilKit
