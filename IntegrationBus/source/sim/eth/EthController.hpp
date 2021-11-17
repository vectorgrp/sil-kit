// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/eth/IEthController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "IIbToEthController.hpp"
#include "IComAdapterInternal.hpp"

#include <memory>

namespace ib {
namespace sim {
namespace eth {

class EthController
    : public IEthController
    , public IIbToEthController
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    EthController() = delete;
    EthController(const EthController&) = default;
    EthController(EthController&&) = default;
    EthController(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    EthController& operator=(EthController& other) = default;
    EthController& operator=(EthController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override;
    void Deactivate() override;

    [[deprecated("For MDF4 support, you should migrate to the SendFrame(...) API")]]
    auto SendMessage(EthMessage msg) -> EthTxId override;

    auto SendFrame(EthFrame msg) -> EthTxId override;
    auto SendFrame(EthFrame msg, std::chrono::nanoseconds timestamp) -> EthTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthMessage& msg) override;
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthTransmitAcknowledge& msg) override;

    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;

    // ib::mw::sync::ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider*) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

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

    inline auto MakeTxId() -> EthTxId;

private:
    // ----------------------------------------
    // private members
    ::ib::mw::IComAdapterInternal* _comAdapter = nullptr;
    ::ib::mw::EndpointAddress _endpointAddr;
    ::ib::mw::sync::ITimeProvider* _timeProvider{ nullptr };

    EthTxId _ethTxId = 0;

    std::tuple<
        CallbackVector<EthMessage>,
        CallbackVector<EthTransmitAcknowledge>
    > _callbacks;

    extensions::Tracer _tracer;

    std::vector<std::pair<EthMac, EthTxId>> _pendingAcks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto EthController::MakeTxId() -> EthTxId
{
    return ++_ethTxId;
}

void EthController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

} // namespace eth
} // namespace sim
} // namespace ib
