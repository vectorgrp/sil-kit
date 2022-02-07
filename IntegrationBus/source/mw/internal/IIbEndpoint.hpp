// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {

//[[deprecated] will be replaced by IIbServiceEndpoint , IIbReceiver will be directly implemented on corresponding Service/Controller
template<typename... MsgT>
class IIbEndpoint : public IIbReceiver<MsgT...>
{
public:
    virtual ~IIbEndpoint() = default;
};

} // namespace mw
} // namespace ib
