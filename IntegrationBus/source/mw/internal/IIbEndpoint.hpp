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

    //! \brief Setter and getter for the EndpointAddress associated with this controller
    virtual void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress) = 0;
    virtual auto EndpointAddress() const -> const ib::mw::EndpointAddress& = 0;
};

} // namespace mw
} // namespace ib
