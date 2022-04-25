// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthControllerFacade interface
 *
 *  Used by the Participant, implemented by the EthControllerFacade
 */
class IIbToEthControllerFacade
    : public mw::IIbReceiver<EthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
    , public mw::IIbSender<EthernetFrameEvent, EthernetSetMode>
{
public:
    virtual ~IIbToEthControllerFacade() noexcept = default;
};

} // namespace eth
} // namespace sim
} // namespace ib
