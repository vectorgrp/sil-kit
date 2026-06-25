// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "core/internal/IServiceEndpoint.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"

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
