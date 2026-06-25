// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "services/metrics/IMetricsSink.hpp"
#include "services/metrics/IMetricsSender.hpp"
#include "services/logging/ILoggerInternal.hpp"
#include "config/ParticipantConfiguration.hpp"

#include <memory>
#include <string>
#include <vector>

namespace VSilKit {

auto CreateMetricsSinksFromParticipantConfiguration(
    SilKit::Services::Logging::ILoggerInternal* logger, IMetricsSender* sender, const std::string& participantName,
    const std::vector<SilKit::Config::MetricsSink>& configuredSinks) -> std::vector<std::unique_ptr<IMetricsSink>>;

} // namespace VSilKit
