// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "silkit/services/eth/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

/*! \brief IMsgForEthController interface
 *
 *  Used by the Participant, implemented by the EthController
 */
class IMsgForEthController
    : public Core::IReceiver<EthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
    , public Core::ISender<EthernetFrameEvent, EthernetSetMode>
{
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
