// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/data/IDataSubscriber.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToDataSubscriber.hpp"
#include "IParticipantInternal.hpp"
#include "DataSubscriberInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriber
    : public IDataSubscriber
    , public IIbToDataSubscriber
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    DataSubscriber(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider, const std::string& topic,
                   const std::string& mediaType, const std::map<std::string, std::string>& labels,
                   DataMessageHandlerT defaultDataHandler, NewDataPublisherHandlerT newDataSourceHandler);

public:
    void RegisterServiceDiscovery();

    void SetDefaultDataMessageHandler(DataMessageHandlerT callback) override;

    void AddExplicitDataMessageHandler(DataMessageHandlerT callback, const std::string& mediaType,
                                       const std::map<std::string, std::string>& labels) override;

    void AddInternalSubscriber(const std::string& pubUUID, const std::string& joinedMediaType,
                               const std::map<std::string, std::string>& publisherLabels);

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;
    
    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:

    void AssignSpecificDataHandlers();

    std::string _topic;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    DataMessageHandlerT _defaultDataHandler;
    NewDataPublisherHandlerT _newDataSourceHandler;

    mw::ServiceDescriptor _serviceDescriptor{};

    std::vector<DataSubscriberInternal*> _internalSubscibers;
    uint64_t _specificDataHandlerId{ 0 };
    std::vector<SpecificDataHandler> _specificDataHandling;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriber::SetServiceDescriptor(const ib::mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriber::GetServiceDescriptor() const -> const ib::mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace data
} // namespace sim
} // namespace ib
