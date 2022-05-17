// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "VAsioPeerInfo.hpp"
#include "VAsioDatatypes.hpp"

namespace ib {
namespace mw {

class MessageBuffer;

class IVAsioPeer
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    virtual ~IVAsioPeer() = default;

public:
    // ----------------------------------------
    // Public interface methods
    virtual void SendIbMsg(MessageBuffer buffer) = 0;
    virtual void Subscribe(VAsioMsgSubscriber subscriber) = 0;

    virtual auto GetInfo() const -> const VAsioPeerInfo& = 0;
    virtual void SetInfo(VAsioPeerInfo info) = 0;
    //! Remote socket endpoint address.
    virtual auto GetRemoteAddress() const -> std::string = 0;
    //! Local socket endpoint address.
    virtual auto GetLocalAddress() const -> std::string = 0;
    //< Start the reading in the IO loop context
    virtual void StartAsyncRead() = 0;

    // TODO cleanup
    //! Handshake and Version management for backward compatibility on network ser/des level
    enum class ProtocolState {
        Invalid,
        DoHandshake,
        HandshakeTimeout,
        HandshakeComplete,
    };
    ProtocolState _state{ProtocolState::Invalid};
    uint32_t _protocolVersion{0};
    virtual void SetProtocolState(ProtocolState state)  {
        _state = state;
    }
    virtual auto GetProtocolState() const -> ProtocolState {
        return _state;
    }
    virtual void SetProtocolVersion(uint32_t v) {
        _protocolVersion= v;
    }
    virtual auto GetProtocolVersion() -> uint32_t {
        return _protocolVersion;
    }

};

} // mw
} // namespace ib
