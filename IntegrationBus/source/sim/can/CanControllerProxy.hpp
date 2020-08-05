// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>

#include "ib/sim/can/ICanController.hpp"
#include "ib/sim/can/IIbToCanControllerProxy.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "Tracing.hpp"

namespace ib {
namespace sim {
namespace can {

class CanControllerProxy
    : public ICanController
    , public IIbToCanControllerProxy
    , public tracing::IControllerToTraceSink
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    CanControllerProxy() = delete;
    CanControllerProxy(const CanControllerProxy&) = default;
    CanControllerProxy(CanControllerProxy&&) = default;
    CanControllerProxy(mw::IComAdapter* comAdapter);


public:
    // ----------------------------------------
    // Operator Implementations
    CanControllerProxy& operator=(CanControllerProxy& other) = default;
    CanControllerProxy& operator=(CanControllerProxy&& other) = default;

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
    void ReceiveIbMessage(mw::EndpointAddress from, const sim::can::CanMessage& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const sim::can::CanControllerStatus& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ITraceMessageSource
    inline void AddSink(tracing::ITraceMessageSink* sink) override;

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
    void ChangeControllerMode(CanControllerState state);

    template<typename MsgT>
    void RegisterHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> CanTxId;

private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter;
    mw::EndpointAddress _endpointAddr;

    CanTxId _canTxId = 0;
    CanControllerState _controllerState = CanControllerState::Uninit;
    CanErrorState _errorState = CanErrorState::NotAvailable;
    CanConfigureBaudrate _baudRate = { 0, 0 };

    std::tuple<
        CallbackVector<CanMessage>,
        CallbackVector<CanControllerState>,
        CallbackVector<CanErrorState>,
        CallbackVector<CanTransmitAcknowledge>
    > _callbacks;

    tracing::Tracer<CanMessage> _tracer;
    std::map<CanTxId, CanMessage> _transmittedMessages;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto CanControllerProxy::MakeTxId() -> CanTxId
{
    return ++_canTxId;
}

void CanControllerProxy::AddSink(tracing::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

} // namespace can
} // namespace sim
} // namespace ib
