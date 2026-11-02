// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "core/vasio/PeerMetrics.hpp"


namespace VSilKit {
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

void NoMetrics::RxBytes(size_t)
{
    // no op
}

void NoMetrics::TxBytes(size_t)
{
    // no op
}

void NoMetrics::TxQueueSize(size_t)
{
    // no op
}


// PeerMetrics

void PeerMetrics::InitializeMetrics(VSilKit::IMetricsManager* manager, SilKit::Core::IVAsioPeer* peer)
{
    if (_initialized)
    {
        return;
    }

    auto&& remoteParticipant = peer->GetServiceDescriptor().GetParticipantName();
    auto&& simulationName = peer->GetSimulationName();

    _txBytes = manager->GetCounter({"Peer", simulationName, remoteParticipant, "tx_bytes", "[bytes]"});
    _txPackets = manager->GetCounter({"Peer", simulationName, remoteParticipant, "tx_packets", "[count]"});
    _txBandwidth = manager->GetStatistic({"Peer", simulationName, remoteParticipant, "tx_bandwidth", "[Bps]"});

    _rxBytes = manager->GetCounter({"Peer", simulationName, remoteParticipant, "rx_bytes", "[bytes]"});
    _rxPackets = manager->GetCounter({"Peer", simulationName, remoteParticipant, "rx_packets", "[count]"});
    _txQueueSize = manager->GetStatistic({"Peer", simulationName, remoteParticipant, "tx_queue_size", "[count]"});
    _rxBandwidth = manager->GetStatistic({"Peer", simulationName, remoteParticipant, "rx_bandwidth", "[Bps]"});

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

void PeerMetrics::RxBytes(size_t numBytes)
{
    if (!_initialized)
    {
        return;
    }
    _rxBytes->Add(numBytes);
    _rxBandwidth->Take(static_cast<double>(numBytes));
}

void PeerMetrics::TxBytes(size_t numBytes)
{
    if (!_initialized)
    {
        return;
    }
    _txBytes->Add(numBytes);
    _txBandwidth->Take(static_cast<double>(numBytes));
}

void PeerMetrics::TxQueueSize(size_t queueSize)
{
    if (!_initialized)
    {
        return;
    }
    _txQueueSize->Take(static_cast<double>(queueSize));
}


} // namespace VSilKit
