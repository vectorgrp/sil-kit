// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/data/IDataSubscriber.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToDataSubscriberInternal.hpp"
#include "IComAdapterInternal.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriberInternal
    : public IDataSubscriber
    , public IIbToDataSubscriberInternal
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    DataSubscriberInternal() = delete;
    DataSubscriberInternal(const DataSubscriberInternal&) = default;
    DataSubscriberInternal(DataSubscriberInternal&&) = default;

    DataSubscriberInternal(mw::IComAdapterInternal* comAdapter, cfg::DataPort config,
        mw::sync::ITimeProvider* timeProvider, CallbackExchangeFormatT callback);

public:
    // ----------------------------------------
    // Operator Implementations
    DataSubscriberInternal& operator=(DataSubscriberInternal& other) = default;
    DataSubscriberInternal& operator=(DataSubscriberInternal&& other) = default;

public:
    void SetReceiveMessageHandler(CallbackExchangeFormatT callback) override;

    auto Config() const -> const cfg::DataPort& override;

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessage& msg) override;

    void ReceiveMessage(const std::vector<uint8_t>& data);

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

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
    CallbackExchangeFormatT  _callback;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
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
