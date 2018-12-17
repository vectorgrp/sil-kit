// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinControllerProxy interface
 *
 *  Used by the ComAdapter, implemented by the LinControllerProxy
 */
class IIbToLinControllerProxy
    : public mw::IIbEndpoint<LinMessage, TxAcknowledge>
    , public mw::IIbSender<LinMessage, RxRequest, ControllerConfig, SlaveConfiguration, SlaveResponse>
{
    virtual void ReceiveIbMessage(mw::EndpointAddress from, const WakeupRequest& msg) = 0;
};

} // namespace lin
} // namespace sim
} // namespace ib
