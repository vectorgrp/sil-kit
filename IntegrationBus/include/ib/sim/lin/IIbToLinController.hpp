// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinController interface
 *
 *  Used by the ComAdapter, implemented by the LinController
 */
class IIbToLinController
    : public mw::IIbEndpoint<LinMessage, ControllerConfig, SlaveConfiguration, SlaveResponse>
    , public mw::IIbSender<LinMessage, ControllerConfig, SlaveConfiguration, SlaveResponse>
{
    virtual void ReceiveIbMessage(mw::EndpointAddress from, const WakeupRequest& msg) = 0;
};

} // namespace lin
} // namespace sim
} // namespace ib
