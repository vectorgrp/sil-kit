// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/LoggingDatatypes.hpp"

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace logging {

class IIbToLogMsgSender
    : public mw::IIbReceiver<>
    , public mw::IIbSender<LogMsg>
{
};

} // namespace logging
} // namespace mw
} // namespace ib
