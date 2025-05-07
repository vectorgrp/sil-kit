// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForMetricsReceiver.hpp"

#include "LoggerMessage.hpp"
#include "IServiceEndpoint.hpp"
#include "IParticipantInternal.hpp"


namespace VSilKit {


struct IMetricsReceiverListener
{
    virtual ~IMetricsReceiverListener() = default;
    virtual void OnMetricsUpdate(const std::string& simulationName, const std::string& participantName,
                                 const MetricsUpdate& metricsUpdate) = 0;
};


class MetricsReceiver
    : public IMsgForMetricsReceiver
    , public SilKit::Core::IServiceEndpoint
{
public:
    MetricsReceiver(SilKit::Core::IParticipantInternal*, IMetricsReceiverListener& listener);

    // NB: The first constructor argument is present to enable using the CreateController function template. It is
    //     allowed to be nullptr.

public: // IMsgForMetricsReceiver
    void ReceiveMsg(const SilKit::Core::IServiceEndpoint* from, const MetricsUpdate& msg) override;

public: // IServiceEndpoint
    void SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor& override;

private:
    IMetricsReceiverListener* _listener{nullptr};

    SilKit::Core::ServiceDescriptor _serviceDescriptor;
};


} // namespace VSilKit
