// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthController interface
 *
 *  Used by the ComAdapter, implemented by the EthController
 */
class IIbToEthController
    : public ib::mw::IIbEndpoint<EthMessage>
    , public ib::mw::IIbSender<EthMessage>
{
};

} // namespace eth
} // namespace sim
} // namespace ib
