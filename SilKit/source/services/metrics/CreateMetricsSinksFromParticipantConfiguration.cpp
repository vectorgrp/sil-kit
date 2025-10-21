// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateMetricsSinksFromParticipantConfiguration.hpp"

#include "MetricsJsonSink.hpp"
#include "MetricsRemoteSink.hpp"

#include "Assert.hpp"
#include "StringHelpers.hpp"
#include "LoggerMessage.hpp"

#include <fstream>

#include "fmt/format.h"

namespace VSilKit {

namespace Log = SilKit::Services::Logging;

auto CreateMetricsSinksFromParticipantConfiguration(
    SilKit::Services::Logging::ILogger* logger, IMetricsSender* sender, const std::string& participantName,
    const std::vector<SilKit::Config::MetricsSink>& configuredSinks) -> std::vector<std::unique_ptr<IMetricsSink>>
{
    std::vector<std::unique_ptr<IMetricsSink>> sinks;

    auto metricsFileTimestamp = SilKit::Util::CurrentTimestampString();

    for (const auto& config : configuredSinks)
    {
        std::unique_ptr<IMetricsSink> sink;

        if (config.type == SilKit::Config::MetricsSink::Type::JsonFile)
        {
            auto filename = fmt::format("{}_{}.txt", config.name, metricsFileTimestamp);
            auto ostream = std::make_unique<std::ofstream>(filename);
            auto realSink = std::make_unique<MetricsJsonSink>(std::move(ostream));
            sink = std::move(realSink);
        }

        if (config.type == SilKit::Config::MetricsSink::Type::Remote)
        {
            SILKIT_ASSERT(sender != nullptr);

            auto realSink = std::make_unique<MetricsRemoteSink>(participantName, *sender);
            sink = std::move(realSink);
        }

        if (sink == nullptr)
        {
            Log::Error(logger, "Failed to create metrics sink {}", config.name);
            continue;
        }

        sinks.emplace_back(std::move(sink));
    }

    return sinks;
}

} // namespace VSilKit
