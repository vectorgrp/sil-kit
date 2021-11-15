// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/io/IoDatatypes.hpp"

namespace ib {
namespace sim {
namespace io {
    
template<typename MsgT>
class IIbToInPort
    : public mw::IIbEndpoint<MsgT>
    , public mw::IIbSender<>
{
};
    
} // namespace io
} // namespace sim
} // namespace ib
