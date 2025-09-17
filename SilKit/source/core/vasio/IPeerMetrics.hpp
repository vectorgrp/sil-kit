// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "IMetricsManager.hpp"
#include "IVAsioPeer.hpp"
#include "SerializedMessage.hpp"

namespace VSilKit {

struct IPeerMetrics
{
    virtual ~IPeerMetrics() = default;
    virtual void InitializeMetrics(VSilKit::IMetricsManager* manager, SilKit::Core::IVAsioPeer* peer) = 0;
    virtual void RxPacket() = 0;
    virtual void TxPacket() = 0;
    virtual void RxBytes(const SilKit::Core::SerializedMessage&) = 0;
    virtual void TxBytes(const SilKit::Core::SerializedMessage&) = 0;
    virtual void TxQueueSize(size_t) = 0;
};

} // namespace VSilKit
