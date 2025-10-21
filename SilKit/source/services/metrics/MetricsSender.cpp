// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsSender.hpp"

#include "LoggerMessage.hpp"


namespace Log = SilKit::Services::Logging;


namespace VSilKit {


MetricsSender::MetricsSender(SilKit::Core::IParticipantInternal* participant)
    : _participant{participant}
    , _logger{participant->GetLogger()}
{
    _serviceDescriptor.SetNetworkName("default");
}


// IMetricsSender

void MetricsSender::Send(const VSilKit::MetricsUpdate& msg)
{
    _participant->SendMsg(this, msg);
}


// IServiceEndpoint

void MetricsSender::SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto MetricsSender::GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}


} // namespace VSilKit
