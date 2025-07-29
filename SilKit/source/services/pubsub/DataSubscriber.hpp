// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "silkit/services/pubsub/IDataSubscriber.hpp"
#include "ITimeConsumer.hpp"

#include "IMsgForDataSubscriber.hpp"
#include "IParticipantInternal.hpp"
#include "DataSubscriberInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "ITraceMessageSource.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriber
    : public IDataSubscriber
    , public IMsgForDataSubscriber
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
    , public ITraceMessageSource
{
public:
    DataSubscriber(Core::IParticipantInternal* participant, Config::DataSubscriber config,
                   Services::Orchestration::ITimeProvider* timeProvider,
                   const SilKit::Services::PubSub::PubSubSpec& dataSpec, DataMessageHandler defaultDataHandler);

public: //methods
    void RegisterServiceDiscovery();
    void SetDataMessageHandler(DataMessageHandler callback) override;

    // SilKit::Services::Orchestration::ITimeConsumer
    inline void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;


    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    //For tracing in DataSubscriberInternal
    inline auto GetConfig() -> const Config::DataSubscriber&
    {
        return _config;
    }

private: //methods
    void AddInternalSubscriber(const std::string& pubUUID, const std::string& joinedMediaType,
                               const std::vector<SilKit::Services::MatchingLabel>& publisherLabels);

    void RemoveInternalSubscriber(const std::string& pubUUID);

    DataMessageHandler WrapTracingCallback(DataMessageHandler callback);

private: //members
    std::string _topic;
    std::string _mediaType;
    std::vector<SilKit::Services::MatchingLabel> _labels;
    Tracer _tracer;

    DataMessageHandler _defaultDataHandler;

    Core::ServiceDescriptor _serviceDescriptor{};

    std::unordered_map<std::string, DataSubscriberInternal*> _internalSubscribers;

    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};

    mutable std::recursive_mutex _internalSubscribersMx;
    Config::DataSubscriber _config;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriber::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

void DataSubscriber::SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriber::GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}


void DataSubscriber::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
