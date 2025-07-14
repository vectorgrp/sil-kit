// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "PeerMetrics.hpp"


namespace VSilKit
{
void NoMetrics::InitializeMetrics(VSilKit::IMetricsManager*, SilKit::Core::IVAsioPeer*)
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

void NoMetrics::TxQueueSize(size_t)
{
    // no op
}


// PeerMetrics

void PeerMetrics::InitializeMetrics(VSilKit::IMetricsManager* manager,
                                    SilKit::Core::IVAsioPeer* peer)
{
    if(_initialized)
    {
        return;
    }

    auto&& remoteParticipant = peer->GetServiceDescriptor().GetParticipantName();
    auto&& simulationName = peer->GetSimulationName();

    _txBytes = manager->GetCounter({"Peer", simulationName, remoteParticipant, "tx_bytes"});
    _txPackets = manager->GetCounter({"Peer", simulationName, remoteParticipant, "tx_packets"});
    _rxBytes = manager->GetCounter({"Peer", simulationName, remoteParticipant, "rx_bytes"});
    _rxPackets = manager->GetCounter({"Peer", simulationName, remoteParticipant, "rx_packets"});
    _txQueueSize = manager->GetCounter({"Peer", simulationName, remoteParticipant, "tx_queue_size"});

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

void PeerMetrics::TxQueueSize(size_t queueSize)
{
    if (!_initialized)
    {
        return;
    }
    _txQueueSize->Add(queueSize);
}


}
