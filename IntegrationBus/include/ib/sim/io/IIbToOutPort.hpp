// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

#include "IoDatatypes.hpp"

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
