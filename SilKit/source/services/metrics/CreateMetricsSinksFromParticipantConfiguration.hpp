// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsSink.hpp"
#include "IMetricsSender.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "ParticipantConfiguration.hpp"

#include <memory>
#include <string>
#include <vector>

namespace VSilKit {

auto CreateMetricsSinksFromParticipantConfiguration(
    SilKit::Services::Logging::ILogger* logger, IMetricsSender* sender, const std::string& participantName,
    const std::vector<SilKit::Config::MetricsSink>& configuredSinks) -> std::vector<std::unique_ptr<IMetricsSink>>;

} // namespace VSilKit