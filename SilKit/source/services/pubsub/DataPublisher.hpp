/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
    DataPublisher(Core::IParticipantInternal* participant,
                  Services::Orchestration::ITimeProvider* timeProvider,
                  const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                  const std::string& pubUUID,
                  const Config::DataPublisher& config  
    );


public: // Methods
    void Publish(Util::Span<const uint8_t> data) override;

    //SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    auto GetTracer() -> Tracer*;

    // IReplayDataController
    void ReplayMessage(const SilKit::IReplayMessage *message) override;
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
