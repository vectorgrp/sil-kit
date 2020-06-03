// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>

#include "ib/sim/fr/IFrController.hpp"
#include "ib/sim/fr/IIbToFrController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "ib/mw/fwd_decl.hpp"


namespace ib {
namespace sim {
namespace fr {

/*! \brief FlexRay Controller implementation for standalone usage
 *
 * Enables FlexRay simulation without a Network Simulator.
 */
class FrController
    : public IFrController
    , public IIbToFrController
    , public mw::sync::ITimeConsumer
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FrController() = delete;
    FrController(const FrController&) = default;
    FrController(FrController&&) = default;
    FrController(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    FrController& operator=(FrController& other) = default;
    FrController& operator=(FrController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Configure(const ControllerConfig& config) override;

    void ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config) override;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * The FlexRay message will be sent immediately and only once.
     * I.e., the configuration according to cycle, repetition, and transmission mode is
     * ignored. In particular, even with TransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFrController::Configure(const ControllerConfig&)
     */
    void UpdateTxBuffer(const TxBufferUpdate& update) override;

    void Run() override;
    void DeferredHalt() override;
    void Freeze() override;
    void AllowColdstart() override;
    void AllSlots() override;
    void Wakeup() override;

    void RegisterMessageHandler(MessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterWakeupHandler(WakeupHandler handler) override;
    void RegisterControllerStatusHandler(ControllerStatusHandler handler) override;
    void RegisterPocStatusHandler(PocStatusHandler handler) override;
    void RegisterSymbolHandler(SymbolHandler handler) override;
    void RegisterSymbolAckHandler(SymbolAckHandler handler) override;
    void RegisterCycleStartHandler(CycleStartHandler handler) override;

    // IIbToFrController
    void ReceiveIbMessage(mw::EndpointAddress from, const FrMessage& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const FrMessageAck& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const FrSymbol& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const FrSymbolAck& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

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

    template<typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddr;
    mw::sync::ITimeProvider* _timeProvider{nullptr};

    ClusterParameters _clusterParams;
    NodeParameters _nodeParams;
    std::vector<TxBufferConfig> _bufferConfigs;

    std::tuple<
        CallbackVector<FrMessage>,
        CallbackVector<FrMessageAck>,
        CallbackVector<FrSymbol>,
        CallbackVector<FrSymbolAck>,
        CallbackVector<ControllerStatus>,
        CallbackVector<PocStatus>
    > _callbacks;

    CallbackVector<FrSymbol> _wakeupHandlers;
};

} // namespace fr
} // SimModels
} // namespace ib
