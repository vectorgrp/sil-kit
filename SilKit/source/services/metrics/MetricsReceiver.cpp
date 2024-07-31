// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsReceiver.hpp"

#include "ILoggerInternal.hpp"


namespace Log = SilKit::Services::Logging;


namespace VSilKit {


MetricsReceiver::MetricsReceiver(SilKit::Core::IParticipantInternal *, SilKit::Services::Logging::ILogger &logger,
                                 IMetricsReceiverListener &listener)
    : _logger{&logger}
    , _listener{&listener}
{
    _serviceDescriptor.SetNetworkName("default");
}


// IMsgForMetricsReceiver

void MetricsReceiver::ReceiveMsg(const SilKit::Core::IServiceEndpoint *from, const VSilKit::MetricsUpdate &msg)
{
    if (_listener == nullptr)
    {
        return;
    }

    _listener->OnMetricsUpdate(from->GetServiceDescriptor().GetParticipantName(), msg);
}


// IServiceEndpoint

void MetricsReceiver::SetServiceDescriptor(const SilKit::Core::ServiceDescriptor &serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto MetricsReceiver::GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor &
{
    return _serviceDescriptor;
}


} // namespace VSilKit
