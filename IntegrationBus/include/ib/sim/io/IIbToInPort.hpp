// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

#include "IoDatatypes.hpp"

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
