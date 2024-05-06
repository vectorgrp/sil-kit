// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "ISimulator.hpp"
#include "SimulatedNetworkRouter.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Lin {

class LinEventProducer : public ILinEventProducer
{
public:
    LinEventProducer(SimulatedNetworkRouter* busSimulator);

    // ILinEventProducer
    void Produce(const SilKit::Services::Lin::LinFrameStatusEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Lin::LinSendFrameHeaderRequest& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

    void Produce(const SilKit::Services::Lin::LinWakeupEvent& msg,
                 const SilKit::Util::Span<const ControllerDescriptor>& receivers) override;

private:
    SimulatedNetworkRouter* _simulatedNetworkRouter;
};

} // namespace Lin
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit