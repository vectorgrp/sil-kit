// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <string>

#include "services/metrics/IMetricsManager.hpp"
#include "core/vasio/IVAsioPeer.hpp"

namespace VSilKit {

struct IPeerMetrics
{
    virtual ~IPeerMetrics() = default;
    virtual void InitializeMetrics(VSilKit::IMetricsManager* manager, SilKit::Core::IVAsioPeer* peer) = 0;
    virtual void RxPacket() = 0;
    virtual void TxPacket() = 0;
    virtual void RxBytes(size_t) = 0;
    virtual void TxBytes(size_t) = 0;
    virtual void TxQueueSize(size_t) = 0;
};

} // namespace VSilKit
