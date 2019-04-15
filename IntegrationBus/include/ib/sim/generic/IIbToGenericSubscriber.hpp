// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

#include "GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

//! \brief IIbToGenericSubscriber interface used by the ComAdapter
class IIbToGenericSubscriber
    : public mw::IIbEndpoint<GenericMessage>
    , public mw::IIbSender<>
{
};

} // namespace generic
} // namespace sim
} // namespace ib

