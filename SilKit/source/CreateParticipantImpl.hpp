// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <memory>
#include <string>


// Forward Declarations

namespace SilKit {
class IParticipant;
namespace Config {
class IParticipantConfiguration;
} // namespace Config
} // namespace SilKit


// Function Declarations

namespace SilKit {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName,
                           const std::string& registryUri) -> std::unique_ptr<IParticipant>;

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName) -> std::unique_ptr<IParticipant>;

} // namespace SilKit
