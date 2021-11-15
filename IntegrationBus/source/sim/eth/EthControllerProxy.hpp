// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <map>

#include "ib/sim/eth/IEthController.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/Config.hpp"

#include "IIbToEthControllerProxy.hpp"

namespace ib {
namespace sim {
namespace eth {


class EthControllerProxy
    : public IEthController
    , public IIbToEthControllerProxy
    , public extensions::ITraceMessageSource
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    EthControllerProxy() = delete;
    EthControllerProxy(const EthControllerProxy&) = default;
    EthControllerProxy(EthControllerProxy&&) = default;
    EthControllerProxy(mw::IComAdapter* comAdapter, cfg::EthernetController config);

public:
    // ----------------------------------------
    // Operator Implementations
    EthControllerProxy& operator=(EthControllerProxy& other) = default;
    EthControllerProxy& operator=(EthControllerProxy&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override;
    void Deactivate() override;

    [[deprecated("For MDF4 support, you should migrate to the SendFrame(...) API")]]
    auto SendMessage(EthMessage msg) -> EthTxId override;

    auto SendFrame(EthFrame frame) -> EthTxId override;
    auto SendFrame(EthFrame frame, std::chrono::nanoseconds timestamp) -> EthTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(mw::EndpointAddress from, const EthMessage& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const EthTransmitAcknowledge& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const EthStatus& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

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
    void RegisterHandler(CallbackT<MsgT>&& handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> EthTxId;

private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter = nullptr;
    mw::EndpointAddress _endpointAddr;

    EthTxId _ethTxId = 0;
    EthState _ethState = EthState::Inactive;
    uint32_t _ethBitRate = 0;

    std::tuple<
        CallbackVector<EthMessage>,
        CallbackVector<EthTransmitAcknowledge>,
        CallbackVector<EthState>,
        CallbackVector<uint32_t>
    > _callbacks;

    extensions::Tracer _tracer;
    std::map<EthTxId, EthFrame> _transmittedMessages;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto EthControllerProxy::MakeTxId() -> EthTxId
{
    return ++_ethTxId;
}

void EthControllerProxy::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

} // namespace eth
} // namespace sim
} // namespace ib
