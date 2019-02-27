// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/eth/IEthController.hpp"
#include "ib/sim/eth/IIbToEthController.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "EthPcapTracer.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthController
    : public IEthController
    , public IIbToEthController
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
    EthController(mw::IComAdapter* comAdapter);

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

    auto SendMessage(EthMessage msg) -> EthTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthMessage& msg) override;
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthTransmitAcknowledge& msg) override;

    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;

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
    ::ib::mw::IComAdapter* _comAdapter = nullptr;
    ::ib::mw::EndpointAddress _endpointAddr;

    EthTxId _ethTxId = 0;

    std::tuple<
        CallbackVector<EthMessage>,
        CallbackVector<EthTransmitAcknowledge>
    > _callbacks;

    ::ib::sim::eth::EthPcapTracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto EthController::MakeTxId() -> EthTxId
{
    return ++_ethTxId;
}

} // namespace eth
} // SimModels
} // namespace ib
