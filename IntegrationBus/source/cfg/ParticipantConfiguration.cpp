// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ParticipantConfiguration.hpp"

#include <sstream>
#include <iomanip>

namespace ib {
namespace cfg {

inline namespace v1 {

// ================================================================================
//  Helper functions
// ================================================================================
namespace {

} // anonymous namespace

// ================================================================================
//  Implementation data types
// ================================================================================
namespace datatypes {

bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const LinController& lhs, const LinController& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const EthernetController& lhs, const EthernetController& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network
        && lhs.macAddress == rhs.macAddress
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const FlexRayController& lhs, const FlexRayController& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network
        && lhs.clusterParameters == rhs.clusterParameters
        && lhs.nodeParameters == rhs.nodeParameters
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const DataPort& lhs, const DataPort& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network
        && lhs.dataExchangeFormat == rhs.dataExchangeFormat
        && lhs.history == rhs.history
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const HealthCheck& lhs, const HealthCheck& rhs)
{
    return lhs.softResponseTimeout == rhs.softResponseTimeout
        && lhs.hardResponseTimeout == rhs.hardResponseTimeout;
}

bool operator==(const Tracing& lhs, const Tracing& rhs)
{
    return lhs.traceSinks == rhs.traceSinks
        && lhs.traceSources == rhs.traceSources;
}

bool operator==(const Extensions& lhs, const Extensions& rhs)
{
    return lhs.searchPathHints == rhs.searchPathHints;
}

bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs)
{
    return lhs.participantName == rhs.participantName
        && lhs.canControllers == rhs.canControllers
        && lhs.linControllers == rhs.linControllers
        && lhs.ethernetControllers == rhs.ethernetControllers
        && lhs.flexRayControllers == rhs.flexRayControllers
        && lhs.dataPublishers == rhs.dataPublishers
        && lhs.dataSubscribers == rhs.dataSubscribers
        && lhs.logging == rhs.logging
        && lhs.healthCheck == rhs.healthCheck
        && lhs.tracing == rhs.tracing
        && lhs.extensions == rhs.extensions;
}

std::ostream& to_ostream(std::ostream& out, const std::array<uint8_t, 6>& macAddress)
{
    //instead of ios::copyfmt (which set badbit) we use a temporary stream 
    std::stringstream buf;
    buf.width(2);
    buf.fill('0');

    auto&& iter = macAddress.begin();
    out << std::hex << std::uppercase << std::setw(2) << static_cast<uint16_t>(*iter++);
    for (; iter != macAddress.end(); ++iter)
    {
        out << ':' << std::hex << std::uppercase << std::setw(2) << static_cast<uint16_t>(*iter);
    }

    return out << buf.str();
}

std::istream& from_istream(std::istream& in, std::array<uint8_t, 6>& macAddress)
{
    for (auto&& iter = macAddress.begin();
        iter != macAddress.end();
        ++iter)
    {
        uint16_t not_a_char;
        in >> std::hex >> not_a_char;
        in.ignore(1, ':');
        *iter = static_cast<uint8_t>(not_a_char);
    }

    return in;
}

} // namespace datatypes

// ================================================================================
//  Public API
// ================================================================================

auto ReadParticipantConfigurationFromYamlString(const std::string& yamlString) -> std::shared_ptr<IParticipantConfiguration>
{
    yamlString;
    throw configuration_error{ "Not implemented" };
}

auto ReadParticipantConfigurationFromFileString(const std::string& yamlFilename) -> std::shared_ptr<IParticipantConfiguration>
{
    yamlFilename;
    throw configuration_error{ "Not implemented" };
}

auto ReadParticipantConfigurationFromJsonString(const std::string& jsonString) -> std::shared_ptr<IParticipantConfiguration>
{
    jsonString;
    throw configuration_error{ "Not implemented" };
}

auto ReadParticipantConfigurationFromJsonFile(const std::string& jsonFilename) -> std::shared_ptr<IParticipantConfiguration>
{
    jsonFilename;
    throw configuration_error{ "Not implemented" };
}

auto ParticipantConfiguration::ToYamlString() -> std::string
{
    throw configuration_error{ "Not implemented" };
}

auto ParticipantConfiguration::ToJsonString() -> std::string
{
    throw configuration_error{ "Not implemented" };
}

#if __cplusplus ==  201402L
// When compiling as C++14, the 'static constexpr variable definitions' are not
// properly exported in the dll (and not properly odr-used from users of the DLL).
// For example, when compiling an extention with GCC and loading the shared
// library, this will result in missing symbols for those networkType definitions.
// Note: In C++17 the static constexpr variable definitions is implicitly inline,
// and the separate definitions in this file are obsolete.
// 
// As a workaround, the missing symbols are defined in this translation
// unit.
constexpr datatypes::NetworkType datatypes::CanController::networkType;
constexpr datatypes::NetworkType datatypes::LinController::networkType;
constexpr datatypes::NetworkType datatypes::EthernetController::networkType;
constexpr datatypes::NetworkType datatypes::FlexRayController::networkType;
constexpr datatypes::NetworkType datatypes::DataPort::networkType;
#endif

} // inline namespace v1

} // namespace cfg
} // namespace ib
