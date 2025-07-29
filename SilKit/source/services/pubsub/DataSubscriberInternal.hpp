// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ITimeConsumer.hpp"

#include "IMsgForDataSubscriberInternal.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "SynchronizedHandlers.hpp"
#include "IReplayDataController.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriberInternal
    : public IMsgForDataSubscriberInternal
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
    , public Tracing::IReplayDataController
{
public: //Ctor
    DataSubscriberInternal(Core::IParticipantInternal* participant,
                           Services::Orchestration::ITimeProvider* timeProvider, const std::string& topic,
                           const std::string& mediaType, const std::vector<SilKit::Services::MatchingLabel>& labels,
                           DataMessageHandler defaultHandler, IDataSubscriber* parent);

public: //Methods
    void SetDataMessageHandler(DataMessageHandler handler);

    //! \brief Accepts messages originating from SilKit communications.
    void ReceiveMsg(const IServiceEndpoint* from, const WireDataMessageEvent& dataMessageEvent) override;

    //SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    std::string GetMediaType()
    {
        return _mediaType;
    };
    auto GetLabels() -> const std::vector<SilKit::Services::MatchingLabel>&
    {
        return _labels;
    };

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    // IReplayDataProvider
    void ReplayMessage(const IReplayMessage* replayMessage) override;

private: //Methods
    void ReceiveInternal(const WireDataMessageEvent& dataMessageEvent);

private: // Member
    std::string _topic;
    std::string _mediaType;
    std::vector<SilKit::Services::MatchingLabel> _labels;
    DataMessageHandler _defaultHandler;

    Config::Replay _replayConfig;

    IDataSubscriber* _parent{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriberInternal::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriberInternal::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
