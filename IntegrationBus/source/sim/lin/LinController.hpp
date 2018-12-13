// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/ILinController.hpp"
#include "ib/sim/lin/IIbToLinController.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController :
    public ILinController,
    public IIbToLinController
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    LinController() = delete;
    LinController(const LinController&) = default;
    LinController(LinController&&) = default;
    LinController(mw::IComAdapter* comAdapter);


public:
    // ----------------------------------------
    // Operator Implementations
    LinController& operator=(LinController& other) = default;
    LinController& operator=(LinController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void SetMasterMode() override;
    void SetSlaveMode() override;
    void SetBaudRate(uint32_t rate) override;

    // LIN Slaves
    void SetSlaveConfiguration(const SlaveConfiguration& config) override;
    void SetResponse(LinId linId, const Payload& payload) override;
    void SetResponseWithChecksum(LinId linId, const Payload& payload, ChecksumModel checksumModel) override;
    void RemoveResponse(LinId linId) override;
    // LIN Masters
    void SendMessage(const LinMessage& msg) override;
    void RequestMessage(const RxRequest& request) override;
    void RegisterTxCompleteHandler(TxCompleteHandler handler) override;
    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;

     // IIbToLinController
     void ReceiveIbMessage(mw::EndpointAddress from, const LinMessage& msg) override;
     void ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg) override;
     void ReceiveIbMessage(mw::EndpointAddress from, const SlaveConfiguration& msg) override;
     void ReceiveIbMessage(mw::EndpointAddress from, const SlaveResponse& msg) override;

     void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
     auto EndpointAddress() const -> const mw::EndpointAddress& override;

private:
    // ----------------------------------------
    // private data types
    template<typename MsgT>
    using CallbackVector = std::vector<CallbackT<MsgT>>;

    struct Response : SlaveResponseConfig
    {
        Payload payload;
    };

    struct LinSlave
    {
        ControllerConfig config;
        std::vector<Response> responses;
    };

private:
    // ----------------------------------------
    // private methods
    template<typename MsgT>
    void RegisterHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

    inline auto GetLinSlave(mw::EndpointAddress addr) -> LinSlave&;
    inline bool IsKnownSlave(mw::EndpointAddress addr) const;
    
private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter;
    mw::EndpointAddress _endpointAddr;

    ControllerMode _controllerMode = ControllerMode::Inactive;
    
    std::tuple<
        CallbackVector<MessageStatus>,
        CallbackVector<LinMessage>
    > _callbacks;

    std::unordered_map<uint32_t, LinSlave> _linSlaves;

    static_assert(
        sizeof(mw::EndpointAddress::participant) + sizeof(mw::EndpointAddress::endpoint) == sizeof(decltype(_linSlaves)::key_type),
        "LinController: _linSlave key_type does not fit an EndpointAddress!"
        );
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto LinController::GetLinSlave(mw::EndpointAddress addr) -> LinSlave&
{
    uint32_t slaveKey = (addr.participant << sizeof(mw::EndpointAddress::endpoint) * 8) | addr.endpoint;
    return _linSlaves[slaveKey];
}

bool LinController::IsKnownSlave(mw::EndpointAddress addr) const
{
    uint32_t slaveKey = (addr.participant << sizeof(mw::EndpointAddress::endpoint) * 8) | addr.endpoint;
    return _linSlaves.count(slaveKey) == 1;
}

} // namespace lin
} // namespace sim
} // namespace ib

