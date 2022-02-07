// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/data/IDataSubscriber.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToDataSubscriber.hpp"
#include "IComAdapterInternal.hpp"
#include "DataSubscriberInternal.hpp"

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
    // ----------------------------------------
    // Constructors and Destructor
    DataSubscriber() = delete;
    DataSubscriber(const DataSubscriber&) = default;
    DataSubscriber(DataSubscriber&&) = default;

    DataSubscriber(mw::IComAdapterInternal* comAdapter, cfg::DataPort config,
        mw::sync::ITimeProvider* timeProvider, CallbackExchangeFormatT callback);

public:
    // ----------------------------------------
    // Operator Implementations
    DataSubscriber& operator=(DataSubscriber& other) = default;
    DataSubscriber& operator=(DataSubscriber&& other) = default;

public:
    void SetReceiveMessageHandler(CallbackExchangeFormatT callback) override;

    auto Config() const -> const cfg::DataPort& override;

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const PublisherAnnouncement& msg) override;

    //! \brief Accepts any message.
    void ReceiveMessage(const PublisherAnnouncement& msg);

    void AddInternalSubscriber(const std::string& networkName, DataExchangeFormat joinedDataExchangFormat);

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
    std::vector<DataSubscriberInternal*> _internalSubscibers;

};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriber::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriber::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace data
} // namespace sim
} // namespace ib
