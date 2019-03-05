// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace spdlog {
    struct log_msg;
}

namespace ib {
namespace mw {
namespace logging {

class IIbToLogmsgRouter
    : public mw::IIbEndpoint<LogMsg>
    , public mw::IIbSender<LogMsg>
{
};


} // namespace logging
} // namespace mw
} // namespace ib
