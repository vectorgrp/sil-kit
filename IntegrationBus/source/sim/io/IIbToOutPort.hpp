// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/io/IoDatatypes.hpp"

namespace ib {
namespace sim {
namespace io {
    
template<typename MsgT>
class IIbToOutPort
    : public mw::IIbEndpoint<>
    , public mw::IIbSender<MsgT>
{
};
    
    
} // namespace io
} // namespace sim
} // namespace ib
