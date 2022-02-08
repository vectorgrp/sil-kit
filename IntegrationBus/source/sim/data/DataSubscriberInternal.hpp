// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToDataSubscriberInternal.hpp"
#include "IComAdapterInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriberInternal
    : public IIbToDataSubscriberInternal
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    DataSubscriberInternal(mw::IComAdapterInternal* comAdapter, cfg::DataPort config,
        mw::sync::ITimeProvider* timeProvider, DataHandlerT defaultHandler, IDataSubscriber* parent);

    void SetDefaultReceiveMessageHandler(DataHandlerT handler);

    void RegisterSpecificDataHandlerInternal(DataHandlerT handler);

    auto Config() const -> const cfg::DataPort&;

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessage& msg) override;

    void ReceiveMessage(const std::vector<uint8_t>& data);

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    //private Members
    cfg::DataPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    DataHandlerT  _defaultHandler;
    std::vector<DataHandlerT> _specificHandlers;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    IDataSubscriber* _parent{ nullptr };
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriberInternal::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriberInternal::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace data
} // namespace sim
} // namespace ib
