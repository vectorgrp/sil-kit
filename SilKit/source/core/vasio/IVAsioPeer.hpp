/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <tuple>

#include "IServiceEndpoint.hpp"

#include "VAsioPeerInfo.hpp"
#include "VAsioDatatypes.hpp"
#include "VAsioProtocolVersion.hpp"

#include "SerializedMessage.hpp"

namespace SilKit {
namespace Core {


class IVAsioPeer : public IServiceEndpoint
{
public:
    ~IVAsioPeer() override = default;

public:
    virtual void SendSilKitMsg(SerializedMessage buffer) = 0;
    virtual void Subscribe(VAsioMsgSubscriber subscriber) = 0;

    virtual auto GetInfo() const -> const VAsioPeerInfo& = 0;
    virtual void SetInfo(VAsioPeerInfo info) = 0;

    virtual auto GetSimulationName() const -> const std::string& = 0;
    virtual void SetSimulationName(const std::string& simulationName) = 0;

    //! Remote socket endpoint address.
    virtual auto GetRemoteAddress() const -> std::string = 0;
    //! Local socket endpoint address.
    virtual auto GetLocalAddress() const -> std::string = 0;
    //! Start the reading in the IO loop context
    virtual void StartAsyncRead() = 0;
    //! Stops the IO loop of the peer
    virtual void Shutdown() = 0;
    //! Version management for backward compatibility on network ser/des level
    virtual void SetProtocolVersion(ProtocolVersion v) = 0;
    virtual auto GetProtocolVersion() const -> ProtocolVersion = 0;

    virtual void EnableAggregation() = 0;
};


struct IVAsioPeerListener
{
    virtual ~IVAsioPeerListener() = default;

    virtual void OnSocketData(IVAsioPeer* peer, SerializedMessage&& buffer) = 0;
    virtual void OnPeerShutdown(IVAsioPeer* peer) = 0;
};


} // namespace Core
} // namespace SilKit
