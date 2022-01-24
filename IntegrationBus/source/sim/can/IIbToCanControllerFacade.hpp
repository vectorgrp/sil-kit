// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/can/CanDatatypes.hpp"

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace sim {
namespace can {

/*! \brief IIbToCanControllerFacade interface
 *
 *  Used by the ComAdapter, implemented by the CanControllerFacade
 */
  class IIbToCanControllerFacade
    : public mw::IIbEndpoint<CanMessage, CanTransmitAcknowledge, CanControllerStatus>
    , public mw::IIbSender<CanMessage, CanConfigureBaudrate, CanSetControllerMode>
{
public:
    virtual ~IIbToCanControllerFacade() noexcept = default;
};

} // namespace can
} // namespace sim
} // namespace ib
