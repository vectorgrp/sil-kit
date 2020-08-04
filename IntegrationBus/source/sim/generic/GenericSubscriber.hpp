// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/generic/IGenericSubscriber.hpp"
#include "ib/sim/generic/IIbToGenericSubscriber.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"


namespace ib {
namespace sim {
namespace generic {

class GenericSubscriber
    : public IGenericSubscriber
    , public IIbToGenericSubscriber
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    GenericSubscriber() = delete;
    GenericSubscriber(const GenericSubscriber&) = default;
    GenericSubscriber(GenericSubscriber&&) = default;

    GenericSubscriber(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);
    GenericSubscriber(mw::IComAdapter* comAdapter, cfg::GenericPort config,
        mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    GenericSubscriber& operator=(GenericSubscriber& other) = default;
    GenericSubscriber& operator=(GenericSubscriber&& other) = default;

public:
    void SetReceiveMessageHandler(CallbackT callback)  override;

    auto Config() const -> const cfg::GenericPort& override;

    void ReceiveIbMessage(mw::EndpointAddress from, const GenericMessage& msg) override;
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

private:
    //private Members
    cfg::GenericPort _config{};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddr{};
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

} // namespace generic
} // namespace sim
} // namespace ib
