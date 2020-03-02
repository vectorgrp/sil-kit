// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrControllerProxy interface
 *
 *  Used by the ComAdapter, implemented by the FrControllerProxy
 */
class IIbToFrControllerProxy
    : public ib::mw::IIbEndpoint<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, CycleStart, ControllerStatus, PocStatus>
    , public ib::mw::IIbSender<HostCommand, ControllerConfig, TxBufferConfigUpdate, TxBufferUpdate>
{
};

} // namespace fr
} // namespace sim
} // namespace ib
