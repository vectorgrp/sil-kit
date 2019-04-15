// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/EndpointAddress.hpp"

#include "idl/Common.h"
#include "idl/MiddlewareTopics.h"


namespace ib {
namespace mw {

inline auto to_idl(const ParticipantId& participantId) -> idl::ParticipantIdT;
inline auto to_idl(const EndpointAddress& endpointAddress) -> idl::EndpointAddress;

namespace idl {
inline auto from_idl(const idl::ParticipantIdT& participantId) ->ib::mw::ParticipantId;
inline auto from_idl(const idl::EndpointAddress& idlEndpointAddress) -> ::ib::mw::EndpointAddress;
} // namespace idl

// ================================================================================
//  Inline Implementations
// ================================================================================

inline auto to_idl(const ParticipantId& participantId) -> idl::ParticipantIdT
{
  return participantId;
}

inline auto idl::from_idl(const idl::ParticipantIdT& participantId) -> ::ib::mw::ParticipantId
{
    return participantId;
}

inline auto to_idl(const EndpointAddress& endpointAddress) -> idl::EndpointAddress
{
    idl::EndpointAddress idlEndpointAddress;

    idlEndpointAddress.participantId() = endpointAddress.participant;
    idlEndpointAddress.endpointId() = endpointAddress.endpoint;

    return idlEndpointAddress;
}

inline auto idl::from_idl(const idl::EndpointAddress& idlEndpointAddress) -> ::ib::mw::EndpointAddress
{
    ::ib::mw::EndpointAddress endpointAddress;

    endpointAddress.participant = idlEndpointAddress.participantId();
    endpointAddress.endpoint = idlEndpointAddress.endpointId();

    return endpointAddress;
}

} // namespace mw
} // namespace ib
