// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrController interface
 *
 *  Used by the ComAdapter, implemented by the FrController
 */
class IIbToFrController
    : public ib::mw::IIbEndpoint<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck>
    , public ib::mw::IIbSender<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck>
{
};

} // namespace fr
} // namespace sim
} // namespace ib
