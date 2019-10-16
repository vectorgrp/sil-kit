// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"
#include "ParticipantBuilder_Helper.hpp"

namespace ib {
namespace cfg {

template<class IoPortCfg>
class IoPortBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    using ValueType = decltype(IoPortCfg::initvalue);
    
public:
    IoPortBuilder(ParticipantBuilder *participant, std::string name, PortDirection direction, mw::EndpointId endpointId);

    auto WithLink(const std::string& linkname) -> IoPortBuilder&;
    auto WithLinkId(int16_t linkId) -> IoPortBuilder&;
    auto WithEndpointId(mw::EndpointId id) -> IoPortBuilder&;
    auto WithInitValue(ValueType value) -> IoPortBuilder&;
    auto WithUnit(std::string unitname) -> IoPortBuilder&;

    auto operator->() -> ParticipantBuilder*;

    auto Build() -> IoPortCfg;

private:
    IoPortCfg _port;
    std::string _link;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IoPortCfg>
IoPortBuilder<IoPortCfg>::IoPortBuilder(ParticipantBuilder *participant, std::string name, PortDirection direction, mw::EndpointId endpointId)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _port.name = std::move(name);
    _port.direction = direction;
    _port.endpointId = endpointId;
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::operator->() -> ParticipantBuilder*
{
    return Parent();
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::WithLink(const std::string& linkname) -> IoPortBuilder&
{
    auto&& qualifiedName = ParticipantBuilder_MakeQualifiedName(*this,_port.name);
    auto&& link = ParticipantBuilder_AddOrGetLink(*this, _port.linkType, linkname);
    link.AddEndpoint(qualifiedName);

    return WithLinkId(link.LinkId());
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::WithLinkId(int16_t linkId) -> IoPortBuilder&
{
    _port.linkId = linkId;
    return *this;
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::WithEndpointId(mw::EndpointId id) -> IoPortBuilder&
{
    _port.endpointId = id;
    return *this;
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::WithInitValue(ValueType value) -> IoPortBuilder&
{
    _port.initvalue = std::move(value);
    return *this;
}

template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::WithUnit(std::string unitname) -> IoPortBuilder&
{
    _port.unit = std::move(unitname);
    return *this;
}


template<class IoPortCfg>
auto IoPortBuilder<IoPortCfg>::Build() -> IoPortCfg
{
    if (_port.linkId == -1)
    {
        WithLink(_port.name);
    }
    return std::move(_port);
}

} // namespace cfg
} // namespace ib
