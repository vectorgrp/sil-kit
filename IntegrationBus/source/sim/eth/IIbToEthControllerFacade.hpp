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
    : public mw::IIbReceiver<EthMessage, EthTransmitAcknowledge, EthStatus>
    , public mw::IIbSender<EthMessage, EthSetMode>
{
public:
    virtual ~IIbToEthControllerFacade() noexcept = default;
};

} // namespace eth
} // namespace sim
} // namespace ib
