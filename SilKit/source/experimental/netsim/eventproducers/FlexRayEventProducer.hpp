// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "ISimulator.hpp"
#include "SimulatedNetworkRouter.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Flexray {

class FlexRayEventProducer : public IFlexRayEventProducer
{
public:
    FlexRayEventProducer(SimulatedNetworkRouter* busSimulator);

    // IFlexRayEventProducer

    void Produce(
        const SilKit::Services::Flexray::FlexrayFrameEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(
        const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(
        const SilKit::Services::Flexray::FlexraySymbolEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(
        const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(
        const SilKit::Services::Flexray::FlexrayCycleStartEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(
        const SilKit::Services::Flexray::FlexrayPocStatusEvent& msg,
        const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

private:
    SimulatedNetworkRouter* _simulatedNetworkRouter;
};

} // namespace Flexray
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit