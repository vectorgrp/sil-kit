// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrControllerFacade interface
 *
 *  Used by the ComAdapter, implemented by the FrControllerFacade
 */
class IIbToFrControllerFacade
    : public mw::IIbReceiver<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, CycleStart, PocStatus>
    , public mw::IIbSender</*Fr*/ FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, 
                                /*FrProxy*/ HostCommand, ControllerConfig, TxBufferConfigUpdate, TxBufferUpdate>
{
public:
    virtual ~IIbToFrControllerFacade() noexcept = default;
};

} // namespace fr
} // namespace sim
} // namespace ib
