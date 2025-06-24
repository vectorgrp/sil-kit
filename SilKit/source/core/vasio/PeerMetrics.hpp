// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsManager.hpp"
#include "ICounterMetric.hpp"
#include "IVAsioPeer.hpp"
#include "SerializedMessage.hpp"

namespace VSilKit
{

struct IPeerMetrics
{
    virtual ~IPeerMetrics() = default;
    virtual void InitializeMetrics(const std::string& localParticipantName, VSilKit::IMetricsManager* manager,
                                   SilKit::Core::IVAsioPeer* peer) = 0;
    virtual void RxPacket() = 0;
    virtual void TxPacket() = 0;
    virtual void RxBytes(const SilKit::Core::SerializedMessage&) = 0;
    virtual void TxBytes(const SilKit::Core::SerializedMessage&) = 0;
};

//disable metrics collection 
class NoMetrics final : public IPeerMetrics
{
public:
    void InitializeMetrics(const std::string&, VSilKit::IMetricsManager* , SilKit::Core::IVAsioPeer* ) override
    {
        // no op
    }

    void RxPacket() override
    {
        // no op
    }

    void TxPacket() override
    {
        // no op
    }
    void RxBytes(const SilKit::Core::SerializedMessage& ) override
    {
        // no op
    }
    void TxBytes(const SilKit::Core::SerializedMessage&) override
    {
        // no op
    }
};


// Metric collection for Rx/Tx paths
class PeerMetrics final : public IPeerMetrics
{
private:
    bool _initialized{false};
    VSilKit::ICounterMetric* _rxPackets{nullptr};
    VSilKit::ICounterMetric* _txPackets{nullptr};
    VSilKit::ICounterMetric* _rxBytes{nullptr};
    VSilKit::ICounterMetric* _txBytes{nullptr};

public:
    void InitializeMetrics(const std::string& localParticipantName, VSilKit::IMetricsManager* manager,
                           SilKit::Core::IVAsioPeer* peer) override
    {
        auto&& remoteParticipant = peer->GetServiceDescriptor().GetParticipantName();
        auto&& simulationName = peer->GetSimulationName();
        auto&& prefix = "Peer/" + simulationName + "/" + localParticipantName + "/" + remoteParticipant;

        _txBytes = manager->GetCounter(prefix + "/tx_bytes");
        _txPackets = manager->GetCounter(prefix + "/tx_packets");
        _rxBytes = manager->GetCounter(prefix + "/rx_bytes");
        _rxPackets = manager->GetCounter(prefix + "/rx_packets");

        _initialized = true;
    }

    void RxPacket() override
    {
        if (!_initialized)
        {
            return;
        }
        _rxPackets->Add(1);
    }

    void TxPacket() override
    {
        if (!_initialized)
        {
            return;
        }
        _txPackets->Add(1);
    }
    void RxBytes(const SilKit::Core::SerializedMessage& msg) override
    {
        if (!_initialized)
        {
            return;
        }
        _rxBytes->Add(msg.GetStorageSize());
    }
    void TxBytes(const SilKit::Core::SerializedMessage& msg) override
    {
        if (!_initialized)
        {
            return;
        }
        _txBytes->Add(msg.GetStorageSize());
    }
};

} // namespace VSilKit