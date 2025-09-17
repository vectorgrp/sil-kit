// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include "IPeerMetrics.hpp"
#include "ICounterMetric.hpp"
#include "IStatisticMetric.hpp"
#include "IVAsioPeer.hpp"
#include "SerializedMessage.hpp"

namespace VSilKit {

// Used to disable metrics collection:
class NoMetrics final : public IPeerMetrics
{
public:
    void InitializeMetrics(VSilKit::IMetricsManager*, SilKit::Core::IVAsioPeer*) override;
    void RxPacket() override;
    void TxPacket() override;
    void RxBytes(const SilKit::Core::SerializedMessage&) override;
    void TxBytes(const SilKit::Core::SerializedMessage&) override;
    void TxQueueSize(size_t) override;
};


// Metric collection for Rx/Tx paths
class PeerMetrics final : public IPeerMetrics
{
private:
    bool _initialized{false};
    VSilKit::ICounterMetric* _rxPackets{nullptr};
    VSilKit::ICounterMetric* _txPackets{nullptr};
    VSilKit::ICounterMetric* _rxBytes{nullptr};
    VSilKit::IStatisticMetric* _rxBandwidth{nullptr};
    VSilKit::ICounterMetric* _txBytes{nullptr};
    VSilKit::IStatisticMetric* _txQueueSize{nullptr};
    VSilKit::IStatisticMetric* _txBandwidth{nullptr};

public:
    void InitializeMetrics(VSilKit::IMetricsManager* manager, SilKit::Core::IVAsioPeer* peer) override;
    void RxPacket() override;
    void TxPacket() override;
    void RxBytes(const SilKit::Core::SerializedMessage& msg) override;
    void TxBytes(const SilKit::Core::SerializedMessage& msg) override;
    void TxQueueSize(size_t queueSize) override;
};

} // namespace VSilKit