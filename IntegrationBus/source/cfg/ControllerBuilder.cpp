// Copyright (c) Vector Informatik GmbH. All rights reserved.

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

template<>
auto ControllerBuilder<FlexrayController>::WithClusterParameters(const sim::fr::ClusterParameters& clusterParameters) -> ControllerBuilder&
{
    _controller.clusterParameters = clusterParameters;
    return *this;
}

template<>
auto ControllerBuilder<FlexrayController>::WithNodeParameters(const sim::fr::NodeParameters& nodeParameters) -> ControllerBuilder&
{
    _controller.nodeParameters = nodeParameters;
    return *this;
}

template<>
auto ControllerBuilder<FlexrayController>::WithTxBufferConfigs(const std::vector<sim::fr::TxBufferConfig>& txBufferConfigs) -> ControllerBuilder&
{
    _controller.txBufferConfigs = txBufferConfigs;
    return *this;
}

} // namespace cfg
} // namespace ib

