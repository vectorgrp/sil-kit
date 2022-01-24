// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthControllerFacade interface
 *
 *  Used by the ComAdapter, implemented by the EthControllerFacade
 */
class IIbToEthControllerFacade
    : public ib::mw::IIbEndpoint<EthMessage, EthTransmitAcknowledge, EthStatus>
    , public ib::mw::IIbSender<EthMessage, EthSetMode>
{
public:
    virtual ~IIbToEthControllerFacade() noexcept = default;
};

} // namespace eth
} // namespace sim
} // namespace ib
