// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/can/ICanController.hpp"
#include "ib/sim/can/IIbToCanController.hpp"

#include <tuple>
#include <vector>


namespace ib {
namespace sim {
namespace can {

class CanController :
    public ICanController,
    public IIbToCanController
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
    CanController(mw::IComAdapter* comAdapter);


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
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const sim::can::CanMessage& msg) override;
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) override;

    void SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ::ib::mw::EndpointAddress& override;

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
    ::ib::mw::IComAdapter* _comAdapter;
    ::ib::mw::EndpointAddress _endpointAddr;

    CanTxId _canTxId = 0;

    std::tuple<
        CallbackVector<CanMessage>,
        CallbackVector<CanTransmitAcknowledge>
    > _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto CanController::MakeTxId() -> CanTxId
{
    return ++_canTxId;
}


} // namespace can
} // namespace sim
} // namespace ib
