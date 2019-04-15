// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

#include "idl/GenericTopicsPubSubTypes.h"
#include "idl/GenericTopics.h"

namespace ib {
namespace sim {
namespace generic {

inline auto to_idl(const GenericMessage& msg) -> idl::GenericMessage;
inline auto to_idl(GenericMessage&& msg) -> idl::GenericMessage;
    
namespace idl {
inline auto from_idl(GenericMessage&& idl) -> generic::GenericMessage;
}

// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(const GenericMessage& msg) -> idl::GenericMessage
{
    idl::GenericMessage idl;

    idl.data(msg.data);

    return idl;
}

auto to_idl(GenericMessage&& msg) -> idl::GenericMessage
{
    idl::GenericMessage idl;

    idl.data(std::move(msg.data));

    return idl;
}

auto idl::from_idl(idl::GenericMessage&& idl) -> generic::GenericMessage
{
    generic::GenericMessage msg;

    msg.data = std::move(idl.data());
    
    return msg;
}

} // namespace generic
} // namespace sim
} // namespace ib
