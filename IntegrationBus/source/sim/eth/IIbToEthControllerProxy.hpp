// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthControllerProxy interface
 *
 *  Used by the Participant, implemented by the EthControllerProxy
 */
class IIbToEthControllerProxy
    : public mw::IIbReceiver<EthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
    , public mw::IIbSender<EthernetFrameEvent, EthernetSetMode>
{
};

} // namespace eth
} // namespace sim
} // namespace ib
