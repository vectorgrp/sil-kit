// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <string>

namespace SilKit {
namespace Config {

class IParticipantConfiguration;

auto ParticipantConfigurationFromStringImpl(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

auto ParticipantConfigurationFromFileImpl(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

} // namespace Config
} // namespace SilKit
