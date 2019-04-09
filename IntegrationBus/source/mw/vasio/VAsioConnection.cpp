// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"


#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace mw {

namespace tt = util::tuple_tools;

VAsioConnection::VAsioConnection(cfg::Config config, std::string participantName)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{get_by_name(_config.simulationSetup.participants, _participantName).id}
{
}

void VAsioConnection::joinDomain(uint32_t /*domainId*/)
{
}


} // namespace mw
} // namespace ib
