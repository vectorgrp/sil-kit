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
    : public mw::IIbEndpoint<LinMessage, TxAcknowledge, WakeupRequest>
    , public mw::IIbSender<LinMessage, RxRequest, WakeupRequest, ControllerConfig, SlaveConfiguration, SlaveResponse>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
