// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include <memory>

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"
#include "ReplayBuilder.hpp"

namespace ib {
namespace cfg {

class RpcPortBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI RpcPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId);

    IntegrationBusAPI auto WithLink(const std::string& linkname) -> RpcPortBuilder&;
    IntegrationBusAPI auto WithLinkId(int16_t linkId) -> RpcPortBuilder&;
    IntegrationBusAPI auto WithEndpointId(mw::EndpointId id) -> RpcPortBuilder&;

    IntegrationBusAPI auto WithTraceSink(std::string sinkName) -> RpcPortBuilder&;
    IntegrationBusAPI auto WithReplay(std::string useTraceSource) -> ReplayBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> RpcPort;

private:
    RpcPort _port;
    std::string _link;
    std::unique_ptr<ReplayBuilder> _replayBuilder;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib
