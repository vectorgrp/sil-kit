// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "PeerMetrics.hpp"


namespace VSilKit
{
void NoMetrics::InitializeMetrics(const std::string&, VSilKit::IMetricsManager*, SilKit::Core::IVAsioPeer*)
{
    // no op
}

void NoMetrics::RxPacket()
{
    // no op
}

void NoMetrics::TxPacket()
{
    // no op
}

void NoMetrics::RxBytes(const SilKit::Core::SerializedMessage&)
{
    // no op
}

void NoMetrics::TxBytes(const SilKit::Core::SerializedMessage&)
{
    // no op
}

// PeerMetrics

void PeerMetrics::InitializeMetrics(const std::string& localParticipantName, VSilKit::IMetricsManager* manager,
                                    SilKit::Core::IVAsioPeer* peer)
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

void PeerMetrics::RxPacket()
{
    if (!_initialized)
    {
        return;
    }
    _rxPackets->Add(1);
}

void PeerMetrics::TxPacket()
{
    if (!_initialized)
    {
        return;
    }
    _txPackets->Add(1);
}

void PeerMetrics::RxBytes(const SilKit::Core::SerializedMessage& msg)
{
    if (!_initialized)
    {
        return;
    }
    _rxBytes->Add(msg.GetStorageSize());
}

void PeerMetrics::TxBytes(const SilKit::Core::SerializedMessage& msg)
{
    if (!_initialized)
    {
        return;
    }
    _txBytes->Add(msg.GetStorageSize());
}

}
