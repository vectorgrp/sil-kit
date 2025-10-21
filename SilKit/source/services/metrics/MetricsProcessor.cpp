// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsProcessor.hpp"

#include "IParticipantInternal.hpp"
#include "LoggerMessage.hpp"

namespace Log = SilKit::Services::Logging;

namespace VSilKit {

MetricsProcessor::MetricsProcessor(std::string participantName)
    : _participantName{std::move(participantName)}
{
}

void MetricsProcessor::SetLogger(SilKit::Services::Logging::ILogger& logger)
{
    _logger = &logger;
}

void MetricsProcessor::SetSinks(std::vector<std::unique_ptr<IMetricsSink>> sinks)
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    if (_sinksSetUp)
    {
        Log::Error(_logger, "Refusing to setup metrics sinks again");
        return;
    }

    _sinks = std::move(sinks);

    for (const auto& sink : _sinks)
    {
        for (const auto& [origin, update] : _updateCache)
        {
            sink->Process(origin, update);
        }
    }

    _updateCache.clear();

    _sinksSetUp = true;
}

void MetricsProcessor::Process(const std::string& origin, const VSilKit::MetricsUpdate& metricsUpdate)
{
    if (!_sinksSetUp)
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};

        if (!_sinksSetUp)
        {
            auto& cache = _updateCache[origin].metrics;
            const auto& metrics = metricsUpdate.metrics;

            cache.insert(cache.end(), metrics.begin(), metrics.end());

            return;
        }
    }

    for (const auto& sink : _sinks)
    {
        sink->Process(origin, metricsUpdate);
    }
}

void MetricsProcessor::OnMetricsUpdate(const std::string& /*simulationName*/, const std::string& participantName,
                                       const MetricsUpdate& metricsUpdate)
{
    if (participantName == _participantName)
    {
        return; // ignore metrics received from ourself (avoids infinite loops)
    }

    Process(participantName, metricsUpdate);
}

} // namespace VSilKit
