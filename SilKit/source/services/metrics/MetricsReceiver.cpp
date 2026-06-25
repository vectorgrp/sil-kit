// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/metrics/MetricsReceiver.hpp"

#include "services/logging/LoggerMessage.hpp"


namespace Log = SilKit::Services::Logging;


namespace VSilKit {


MetricsReceiver::MetricsReceiver(SilKit::Core::IParticipantInternal*, IMetricsReceiverListener& listener)
    : _listener{&listener}
{
    _serviceDescriptor.SetNetworkName("default");
}


// IMsgForMetricsReceiver

void MetricsReceiver::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from, const VSilKit::MetricsUpdate& msg)
{
    if (_listener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor = from->GetServiceDescriptor();

    _listener->OnMetricsUpdate(serviceDescriptor.GetSimulationName(), serviceDescriptor.GetParticipantName(), msg);
}


// IServiceEndpoint

void MetricsReceiver::SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto MetricsReceiver::GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}


} // namespace VSilKit
