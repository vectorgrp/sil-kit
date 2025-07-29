// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "WireEthernetMessages.hpp"

#include "silkit/services/ethernet/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

/*! \brief IMsgForEthController interface
 *
 *  Used by the Participant, implemented by the EthController
 */
class IMsgForEthController
    : public Core::IReceiver<WireEthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
    , public Core::ISender<WireEthernetFrameEvent, EthernetSetMode>
{
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
