// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>

#include "VAsioPeerInfo.hpp"
#include "VAsioDatatypes.hpp"
#include "VAsioProtocolVersion.hpp"

#include "SerializedMessage.hpp"

namespace SilKit {
namespace Core {

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
    virtual void SendSilKitMsg(SerializedMessage buffer) = 0;
    virtual void Subscribe(VAsioMsgSubscriber subscriber) = 0;

    virtual auto GetInfo() const -> const VAsioPeerInfo& = 0;
    virtual void SetInfo(VAsioPeerInfo info) = 0;
    //! Remote socket endpoint address.
    virtual auto GetRemoteAddress() const -> std::string = 0;
    //! Local socket endpoint address.
    virtual auto GetLocalAddress() const -> std::string = 0;
    //! Start the reading in the IO loop context
    virtual void StartAsyncRead() = 0;

    //! Version management for backward compatibility on network ser/des level
    virtual void SetProtocolVersion(ProtocolVersion v) = 0;
    virtual auto GetProtocolVersion() const -> ProtocolVersion = 0;
};

} // namespace Core
} // namespace SilKit
