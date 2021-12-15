// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {
namespace service {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ServiceAnnouncement& msg)
{
    //TODO
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceAnnouncement& updatedMsg)
{
    //TODO
    return buffer;
}

} // namespace service
} // namespace mw
} // namespace ib
