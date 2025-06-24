#pragma once

#include <string>

#include "IMetricsManager.hpp"
#include "IVAsioPeer.hpp"
#include "SerializedMessage.hpp"

namespace VSilKit {

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

} // namespace VSilKit
