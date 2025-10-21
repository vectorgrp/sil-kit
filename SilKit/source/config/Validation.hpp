// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Config {

inline namespace V1 {

void Validate(const SilKit::Config::V1::ParticipantConfiguration& configuration);

} // namespace V1

} // namespace Config
} // namespace SilKit
