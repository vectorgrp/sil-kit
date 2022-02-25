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
 *  Used by the ComAdapter, implemented by the EthControllerProxy
 */
class IIbToEthControllerProxy
    : public mw::IIbReceiver<EthMessage, EthTransmitAcknowledge, EthStatus>
    , public mw::IIbSender<EthMessage, EthSetMode>
{
};

} // namespace eth
} // namespace sim
} // namespace ib
