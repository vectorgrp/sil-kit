// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "silkit/services/pubsub/IDataPublisher.hpp"
#include "ITimeConsumer.hpp"

#include "IMsgForDataPublisher.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "IReplayDataController.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataPublisher
    : public IDataPublisher
    , public IMsgForDataPublisher
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
    , public ITraceMessageSource
    , public Tracing::IReplayDataController
{
public:
    DataPublisher(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                  const SilKit::Services::PubSub::PubSubSpec& dataSpec, const std::string& pubUUID,
                  const Config::DataPublisher& config);


public: // Methods
    void Publish(Util::Span<const uint8_t> data) override;

    //SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    auto GetTracer() -> Tracer*;

    // IReplayDataController
    void ReplayMessage(const SilKit::IReplayMessage* message) override;

private: // Methods
    void PublishInternal(Util::Span<const uint8_t> data);

private: // Member
    std::string _topic;
    std::string _mediaType;
    std::vector<SilKit::Services::MatchingLabel> _labels;
    std::string _pubUUID;
    Tracer _tracer;

    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};

    Config::DataPublisher _config;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataPublisher::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

void DataPublisher::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataPublisher::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

inline auto DataPublisher::GetTracer() -> Tracer*
{
    return &_tracer;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
