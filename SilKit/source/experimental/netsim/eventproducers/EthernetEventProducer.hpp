// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "ISimulator.hpp"
#include "SimulatedNetworkRouter.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Ethernet {

class EthernetEventProducer : public IEthernetEventProducer
{
public:
    EthernetEventProducer(SimulatedNetworkRouter* busSimulator);

    // IEthernetEventProducer
    void Produce(const SilKit::Services::Ethernet::EthernetFrameEvent& msg,
                           const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& msg,
                                   const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Ethernet::EthernetStateChangeEvent& msg,
                                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& msg,
                                      const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

private:
    SimulatedNetworkRouter* _simulatedNetworkRouter;

    // Track the state and bitrate because the msg on wire (EthernetStatus) contains both.
    SilKit::Services::Ethernet::EthernetState _ethernetState = SilKit::Services::Ethernet::EthernetState::Inactive;
    SilKit::Services::Ethernet::EthernetBitrate _ethernetBitrate = SilKit::Services::Ethernet::EthernetBitrate{};
};

} // namespace Ethernet
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit