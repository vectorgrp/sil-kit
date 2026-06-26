// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/participant/CreateParticipantInternal.hpp"

#include "core/participant/CreateParticipantT.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantInternal(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName,
                               const std::string& registryUri) -> std::unique_ptr<IParticipantInternal>
{
    return CreateParticipantT<VAsioConnection>(std::move(participantConfig), participantName, registryUri);
}

} // namespace Core
} // namespace SilKit