// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>

#include "IServiceEndpoint.hpp"

#include "VAsioPeerInfo.hpp"
#include "VAsioDatatypes.hpp"
#include "VAsioProtocolVersion.hpp"

#include "SerializedMessage.hpp"
#include "IMetricsManager.hpp"

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

    virtual void InitializeMetrics(VSilKit::IMetricsManager*) = 0;
};


struct IVAsioPeerListener
{
    virtual ~IVAsioPeerListener() = default;

    virtual void OnSocketData(IVAsioPeer* peer, SerializedMessage&& buffer) = 0;
    virtual void OnPeerShutdown(IVAsioPeer* peer) = 0;
};


} // namespace Core
} // namespace SilKit
