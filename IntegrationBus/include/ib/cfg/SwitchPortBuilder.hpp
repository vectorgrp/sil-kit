// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "fwd_decl.hpp"
#include "Config.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {

class SwitchPortBuilder : public ParentBuilder<SwitchBuilder>
{
public:
    IntegrationBusAPI SwitchPortBuilder(SwitchBuilder* parent, std::string name, mw::EndpointId endpointId);

    IntegrationBusAPI auto WithVlanIds(std::initializer_list<uint16_t> vlanIds) -> SwitchPortBuilder&;
    IntegrationBusAPI auto WithEndpointId(mw::EndpointId endpointId) -> SwitchPortBuilder&;
    IntegrationBusAPI auto operator->() -> SwitchBuilder*;

    IntegrationBusAPI auto Build() -> Switch::Port;

private:
    Switch::Port _config;
};

} // namespace cfg
} // namespace ib
