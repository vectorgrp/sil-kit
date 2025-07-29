// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireEthernetMessages.hpp"

#include "silkit/services/ethernet/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

/*! \brief IMsgForEthSimulator interface
*
*  Used by the Participant, implemented by the EthSimulator
*/
class IMsgForEthSimulator
    : public Core::IReceiver<WireEthernetFrameEvent, EthernetSetMode>
    , public Core::ISender<WireEthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
{
public:
    virtual ~IMsgForEthSimulator() = default;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
