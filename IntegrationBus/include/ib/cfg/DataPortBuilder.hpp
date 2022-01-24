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
inline namespace deprecated {

class DataPortBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI DataPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId);

    IntegrationBusAPI auto WithLink(const std::string& linkname) -> DataPortBuilder&;
    IntegrationBusAPI auto WithLinkId(int16_t linkId) -> DataPortBuilder&;
    IntegrationBusAPI auto WithEndpointId(mw::EndpointId id) -> DataPortBuilder&;

    IntegrationBusAPI auto WithTraceSink(std::string sinkName) -> DataPortBuilder&;
    IntegrationBusAPI auto WithReplay(std::string useTraceSource) -> ReplayBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> DataPort;

private:
    DataPort _port;
    std::string _link;
    std::unique_ptr<ReplayBuilder> _replayBuilder;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace deprecated
} // namespace cfg
} // namespace ib
