// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"

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
