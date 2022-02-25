// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrControllerProxy interface
 *
 *  Used by the ComAdapter, implemented by the FrControllerProxy
 */
class IIbToFrControllerProxy
    : public mw::IIbReceiver<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, CycleStart, PocStatus>
    , public mw::IIbSender<HostCommand, ControllerConfig, TxBufferConfigUpdate, TxBufferUpdate>
{
};

} // namespace fr
} // namespace sim
} // namespace ib
