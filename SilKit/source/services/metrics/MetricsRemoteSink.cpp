// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsRemoteSink.hpp"

#include "IParticipantInternal.hpp"
#include "MetricsSender.hpp"

namespace VSilKit {

MetricsRemoteSink::MetricsRemoteSink(std::string participantName, IMetricsSender& sender)
    : _participantName{std::move(participantName)}
    , _sender{&sender}
{
}

void MetricsRemoteSink::Process(const std::string& origin, const VSilKit::MetricsUpdate& metricsUpdate)
{
    // do not forward metrics from other participants again
    if (origin != _participantName)
    {
        return;
    }

    _sender->Send(metricsUpdate);
}

} // namespace VSilKit
