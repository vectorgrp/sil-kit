// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {

template<typename T>
class IIbMessageReceiver
{
public:
    virtual void ReceiveIbMessage(ib::mw::EndpointAddress from, const T& msg) = 0;
};

} // namespace mw
} // namespace ib
