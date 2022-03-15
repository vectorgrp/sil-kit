// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ParticipantConfiguration.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>

#include "YamlParser.hpp"
#include "YamlValidator.hpp"

namespace ib {
namespace cfg {

inline namespace v1 {

// ================================================================================
//  Helper functions
// ================================================================================
namespace {

auto ReadFile(const std::string& filename) -> std::string
{
    std::ifstream fs(filename);

    if (!fs.is_open())
        throw ib::ConfigurationError("the file could not be opened");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return buffer.str();
}

auto Parse(const std::string& text) -> ib::cfg::v1::datatypes::ParticipantConfiguration
{
    std::stringstream warnings;
    ib::cfg::YamlValidator validator;
    if (!validator.Validate(text, warnings))
    {
        throw ib::ConfigurationError{ "YAML validation returned errors: \n" + warnings.str() };
    }
    if (warnings.str().size() > 0)
    {
        std::cout << "YAML validation returned warnings: \n" << warnings.str() << std::endl;
    }
    YAML::Node doc = YAML::Load(text);

    auto configuration = !doc.IsNull() ?
        ib::cfg::from_yaml<ib::cfg::v1::datatypes::ParticipantConfiguration>(doc) :
        ib::cfg::v1::datatypes::ParticipantConfiguration{};
    configuration.configurationFilePath.clear();

    //PostProcess(config);

    return configuration;
}

} // anonymous namespace

// ================================================================================
//  Implementation data types
// ================================================================================
namespace datatypes {

bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name
        && lhs.network == rhs.network;
        //&& lhs.useTraceSinks == rhs.useTraceSinks
        //&& lhs.replay == rhs.replay;
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

bool operator==(const DataPublisher& lhs, const DataPublisher& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks 
        && lhs.replay == rhs.replay;
}

bool operator==(const DataSubscriber& lhs, const DataSubscriber& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks 
        && lhs.replay == rhs.replay;
}

bool operator==(const RpcServer& lhs, const RpcServer& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const RpcClient& lhs, const RpcClient& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks
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

bool operator==(const Registry& lhs, const Registry& rhs)
{
    return lhs.port == rhs.port
        && lhs.hostname == rhs.hostname
        && lhs.logging == rhs.logging
        && lhs.connectAttempts == rhs.connectAttempts;
}

bool operator==(const Middleware& lhs, const Middleware& rhs)
{
    return lhs.registry == rhs.registry
        && lhs.enableDomainSockets == rhs.enableDomainSockets
        && lhs.tcpNoDelay == rhs.tcpNoDelay
        && lhs.tcpQuickAck == rhs.tcpQuickAck
        && lhs.tcpReceiveBufferSize == rhs.tcpReceiveBufferSize
        && lhs.tcpSendBufferSize == rhs.tcpSendBufferSize;
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
        && lhs.rpcClients == rhs.rpcClients
        && lhs.rpcServers == rhs.rpcServers
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
constexpr datatypes::NetworkType datatypes::DataPublisher::networkType;
constexpr datatypes::NetworkType datatypes::DataSubscriber::networkType;
constexpr datatypes::NetworkType datatypes::RpcServer::networkType;
constexpr datatypes::NetworkType datatypes::RpcClient::networkType;
#endif

} // inline namespace v1

auto ParticipantConfigurationFromString(const std::string& text)
-> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    auto configuration = ib::cfg::Parse(text);

    return std::make_shared<ib::cfg::v1::datatypes::ParticipantConfiguration>(std::move(configuration));
}

auto ParticipantConfigurationFromFile(const std::string& filename)
-> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    auto text = ib::cfg::ReadFile(filename);
    auto configuration = ib::cfg::Parse(text);

    return std::make_shared<ib::cfg::v1::datatypes::ParticipantConfiguration>(std::move(configuration));
}

} // namespace cfg
} // namespace ib
