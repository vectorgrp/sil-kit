// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthControllerProxy interface
 *
 *  Used by the ComAdapter, implemented by the EthControllerProxy
 */
class IIbToEthControllerProxy
    : public ib::mw::IIbEndpoint<EthMessage, EthTransmitAcknowledge, EthStatus>
    , public ib::mw::IIbSender<EthMessage, EthSetMode>
{
};

} // namespace eth
} // namespace sim
} // namespace ib
