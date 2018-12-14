// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/ILinController.hpp"
#include "ib/sim/lin/IIbToLinControllerProxy.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

#include "ib/mw/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinControllerProxy :
    public ILinController,
    public IIbToLinControllerProxy
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    LinControllerProxy() = delete;
    LinControllerProxy(const LinControllerProxy&) = default;
    LinControllerProxy(LinControllerProxy&&) = default;
    LinControllerProxy(mw::IComAdapter* comAdapter);

public:
    // ----------------------------------------
    // Operator Implementations
    LinControllerProxy& operator=(LinControllerProxy& other) = default;
    LinControllerProxy& operator=(LinControllerProxy&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void SetMasterMode() override;
    void SetSlaveMode() override;
    void SetBaudRate(uint32_t rate) override;
    void SetSleepMode() override {};
    void SetOperational() override {};

    // LIN Slaves
    void SetSlaveConfiguration(const SlaveConfiguration& config) override;
    void SetResponse(LinId linId, const Payload& payload) override;
    void SetResponseWithChecksum(LinId linId, const Payload& payload, ChecksumModel checksumModel) override;
    void RemoveResponse(LinId linId) override;
    void SendWakeupRequest() override {};

    // LIN Masters
    void SendMessage(const LinMessage& msg) override;
    void RequestMessage(const RxRequest& request) override;
    void SendGoToSleep() override {};

    void RegisterTxCompleteHandler(TxCompleteHandler handler) override;
    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterWakupRequestHandler(WakupRequestHandler handler) override {};
    void RegisterSleepCommandHandler(SleepCommandHandler handler) override {};

     // IIbToLinController
     void ReceiveIbMessage(mw::EndpointAddress from, const LinMessage& msg) override;
     void ReceiveIbMessage(mw::EndpointAddress from, const TxAcknowledge& msg) override;
     
     void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
     auto EndpointAddress() const -> const mw::EndpointAddress& override;

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

    void OnMasterTransmit(const LinMessage& msg);
    void OnSlaveResponse(const LinMessage& msg);

    void sendControllerConfig();

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter;
    mw::EndpointAddress _endpointAddr;

    ControllerMode _controllerMode = ControllerMode::Inactive;
    uint32_t _baudrate = 0;

    std::tuple<
        CallbackVector<MessageStatus>,
        CallbackVector<LinMessage>
    > _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================


} // namespace lin
} // namespace sim
} // namespace ib
