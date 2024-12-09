// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForMetricsSender.hpp"

#include "LoggerMessage.hpp"
#include "IServiceEndpoint.hpp"
#include "IParticipantInternal.hpp"


namespace VSilKit {


class MetricsSender
    : public IMetricsSender
    , public IMsgForMetricsSender
    , public SilKit::Core::IServiceEndpoint
{
public:
    explicit MetricsSender(SilKit::Core::IParticipantInternal*);

    // NB: The first constructor argument is present to enable using the CreateController function template. It is
    //     allowed to be nullptr.

public: // IMetricsSender
    void Send(const MetricsUpdate& msg) override;

public: // IServiceEndpoint
    void SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor& override;

private:
    SilKit::Core::IParticipantInternal* _participant{nullptr};
    SilKit::Services::Logging::ILogger* _logger{nullptr};

    SilKit::Core::ServiceDescriptor _serviceDescriptor;
};


} // namespace VSilKit
