// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/can/ICanController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "IIbToCanController.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

#include <tuple>
#include <vector>

namespace ib {
namespace sim {
namespace can {

class CanController
    : public ICanController
    , public IIbToCanController
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    CanController() = delete;
    CanController(const CanController&) = default;
    CanController(CanController&&) = default;
    CanController(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider);


public:
    // ----------------------------------------
    // Operator Implementations
    CanController& operator=(CanController& other) = default;
    CanController& operator=(CanController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ICanController
    void SetBaudRate(uint32_t rate, uint32_t fdRate) override;

    void Reset() override;
    void Start() override;
    void Stop() override;
    void Sleep() override;

    auto SendMessage(const CanMessage& msg) -> CanTxId override;
    auto SendMessage(CanMessage&& msg) -> CanTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler) override;
    void RegisterTransmitStatusHandler(MessageStatusHandler handler) override;

    // IIbToCanController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg) override;

    void SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ::ib::mw::EndpointAddress& override;

    //ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;
public:
    // ----------------------------------------
    // Public interface methods

private:
    // ----------------------------------------
    // private data types
    template<typename MsgT>
    using CallbackVector = std::vector<CallbackT<MsgT>>;

private:
    // ----------------------------------------
    // private methods
    template<typename MsgT>
    void RegisterHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> CanTxId;

private:
    // ----------------------------------------
    // private members
    ::ib::mw::IComAdapterInternal* _comAdapter{nullptr};
    ::ib::mw::ServiceId _serviceId;
    mw::sync::ITimeProvider* _timeProvider{nullptr};

    CanTxId _canTxId = 0;

    std::tuple<
        CallbackVector<CanMessage>,
        CallbackVector<CanTransmitAcknowledge>
    > _callbacks;

    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto CanController::MakeTxId() -> CanTxId
{
    return ++_canTxId;
}

void CanController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

void CanController::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}
auto CanController::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

} // namespace can
} // namespace sim
} // namespace ib
