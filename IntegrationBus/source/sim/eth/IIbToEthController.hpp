// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthController interface
 *
 *  Used by the Participant, implemented by the EthController
 */
class IIbToEthController
    : public mw::IIbReceiver<EthernetFrameEvent>
    , public mw::IIbSender<EthernetFrameEvent>
{
};

} // namespace eth
} // namespace sim
} // namespace ib
