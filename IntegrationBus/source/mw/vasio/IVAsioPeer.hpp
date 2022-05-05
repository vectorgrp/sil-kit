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
    virtual void SetUri(VAsioPeerUri info) = 0;
    virtual auto GetUri() const -> const VAsioPeerUri& = 0;
    //! Remote socket endpoint address.
    virtual auto GetRemoteAddress() const -> std::string = 0;
    //! Local socket endpoint address.
    virtual auto GetLocalAddress() const -> std::string = 0;

};

//! ProtocolPeer exposes some of the protocol details for testing purposes.
class IVasioProtocolPeer
{
public:
    virtual ~IVasioProtocolPeer() = default;

    //! Signal the peer that the protocl version is not supported
    virtual void VersionNotSupported() = 0;

};

} // mw
} // namespace ib
