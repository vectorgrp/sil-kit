// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "ISimulator.hpp"
#include "SimulatedNetworkRouter.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Can {

class CanEventProducer : public ICanEventProducer
{
public:
    CanEventProducer(SimulatedNetworkRouter* busSimulator);

    // ICanEventProducer
    void Produce(const SilKit::Services::Can::CanFrameEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Can::CanFrameTransmitEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Can::CanStateChangeEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Can::CanErrorStateChangeEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

private:
    SimulatedNetworkRouter* _simulatedNetworkRouter;
    
    // Track the controller state and error state because the msg on wire (CanControllerStatus) contains both.
    SilKit::Services::Can::CanControllerState _controllerState = SilKit::Services::Can::CanControllerState::Uninit;
    SilKit::Services::Can::CanErrorState _errorState = SilKit::Services::Can::CanErrorState::NotAvailable;
};

} // namespace Can
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit