// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SwitchPortBuilder.hpp"

namespace ib {
namespace cfg {


SwitchPortBuilder::SwitchPortBuilder(SwitchBuilder* parent, std::string name, mw::EndpointId endpointId)
    : ParentBuilder<SwitchBuilder>{parent}
{
    _config.name = std::move(name);
    _config.endpointId = endpointId;
}

auto SwitchPortBuilder::WithVlanIds(std::initializer_list<uint16_t> vlanIds) -> SwitchPortBuilder&
{
    _config.vlanIds.insert(_config.vlanIds.end(), vlanIds.begin(), vlanIds.end());
    return *this;
}

auto SwitchPortBuilder::WithEndpointId(mw::EndpointId endpointId) -> SwitchPortBuilder&
{
    _config.endpointId = endpointId;
    return *this;
}

auto SwitchPortBuilder::operator->() -> SwitchBuilder*
{
    return Parent();
}

auto SwitchPortBuilder::Build() -> Switch::Port
{
    return std::move(_config);
}


} // namespace cfg
} // namespace ib
