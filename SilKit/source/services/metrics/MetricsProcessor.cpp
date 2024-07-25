// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsProcessor.hpp"

#include "IParticipantInternal.hpp"
#include "MetricsJsonFileSink.hpp"
#include "MetricsRemoteSink.hpp"
#include "ILogger.hpp"

#include "silkit/util/Span.hpp"

#include "fmt/chrono.h"

namespace Log = SilKit::Services::Logging;

namespace VSilKit {

namespace {

auto CurrentTimestamp() -> std::string
{
    auto time = std::time(nullptr);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tmBuffer, &timeNow);
#else
    localtime_r(&time, &tm);
#endif

    return fmt::format("{:%FT%H-%M-%S}", tm);
}

} // namespace

MetricsProcessor::MetricsProcessor(std::string participantName)
    : _participantName{std::move(participantName)}
{
}

void MetricsProcessor::SetLogger(SilKit::Services::Logging::ILogger &logger)
{
    _logger = &logger;
}

void MetricsProcessor::SetSender(IMetricsSender &sender)
{
    _sender = &sender;
}

void MetricsProcessor::SetupSinks(const SilKit::Config::ParticipantConfiguration &participantConfiguration)
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    if (_sinksSetUp)
    {
        Log::Error(_logger, "Refusing to setup metrics sinks again");
        return;
    }

    auto timestamp = CurrentTimestamp();

    for (const auto &config : participantConfiguration.experimental.metrics.sinks)
    {
        std::unique_ptr<IMetricsSink> sink;

        if (config.type == SilKit::Config::MetricsSink::Type::JsonFile)
        {
            auto filename = fmt::format("{}_{}.txt", config.name, timestamp);
            auto realSink = std::make_unique<MetricsJsonFileSink>(filename);
            sink = std::move(realSink);
        }

        if (config.type == SilKit::Config::MetricsSink::Type::Remote)
        {
            auto realSink = std::make_unique<MetricsRemoteSink>(_participantName, *_sender);
            sink = std::move(realSink);
        }

        if (sink == nullptr)
        {
            Log::Error(_logger, "Failed to create metrics sink {}", config.name);
            continue;
        }

        for (const auto &pair : _updateCache)
        {
            const auto &origin = pair.first;
            const auto &update = pair.second;
            sink->Process(origin, update);
        }

        _sinks.emplace_back(std::move(sink));
    }

    _registrationCache.clear();
    _updateCache.clear();

    _sinksSetUp = true;
}

void MetricsProcessor::Process(const std::string &origin, const VSilKit::MetricsUpdate &metricsUpdate)
{
    if (!_sinksSetUp)
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};

        if (!_sinksSetUp)
        {
            auto &cache = _updateCache[origin].metrics;
            const auto &metrics = metricsUpdate.metrics;

            cache.insert(cache.end(), metrics.begin(), metrics.end());

            return;
        }
    }

    for (const auto &sink : _sinks)
    {
        sink->Process(origin, metricsUpdate);
    }
}

void MetricsProcessor::OnMetricsUpdate(const std::string &participantName, const MetricsUpdate &metricsUpdate)
{
    Process(participantName, metricsUpdate);
}

} // namespace VSilKit
