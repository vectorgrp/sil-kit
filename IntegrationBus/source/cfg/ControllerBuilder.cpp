// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/ControllerBuilder.hpp"

#include <sstream>

namespace ib {
namespace cfg {

template<>
auto ControllerBuilder<EthernetController>::WithMacAddress(std::string macAddress) -> ControllerBuilder&
{
    std::stringstream macIn(macAddress);
    from_istream(macIn, _controller.macAddress);
    return *this;
}

} // namespace cfg
} // namespace ib

