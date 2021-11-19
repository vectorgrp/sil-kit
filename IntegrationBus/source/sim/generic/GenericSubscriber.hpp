// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/generic/IGenericSubscriber.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToGenericSubscriber.hpp"
#include "IComAdapterInternal.hpp"
#include "IServiceId.hpp"

namespace ib {
namespace sim {
namespace generic {

class GenericSubscriber
    : public IGenericSubscriber
    , public IIbToGenericSubscriber
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IServiceId
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    GenericSubscriber() = delete;
    GenericSubscriber(const GenericSubscriber&) = default;
    GenericSubscriber(GenericSubscriber&&) = default;

    GenericSubscriber(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider);
    GenericSubscriber(mw::IComAdapterInternal* comAdapter, cfg::GenericPort config,
        mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    GenericSubscriber& operator=(GenericSubscriber& other) = default;
    GenericSubscriber& operator=(GenericSubscriber&& other) = default;

public:
    void SetReceiveMessageHandler(CallbackT callback)  override;

    auto Config() const -> const cfg::GenericPort& override;

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IServiceId* from, const GenericMessage& msg) override;

    //! \brief Accepts any message, e.g. also from trace replays.
    void ReceiveMessage(const GenericMessage& msg);

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;
    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

private:
    //private Members
    cfg::GenericPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceId _serviceId{};
    CallbackT _callback;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void GenericSubscriber::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

void GenericSubscriber::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto GenericSubscriber::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

} // namespace generic
} // namespace sim
} // namespace ib
