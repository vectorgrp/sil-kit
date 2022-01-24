// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "MiddlewareConfiguration.hpp"

namespace ib {
namespace cfg {
namespace vasio {

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

bool operator==(const Registry& lhs, const Registry& rhs)
{
    return lhs.port == rhs.port
        && lhs.hostname == rhs.hostname
        && lhs.logging == rhs.logging
        && lhs.connectAttempts == rhs.connectAttempts;
}

bool operator==(const MiddlewareConfiguration& lhs, const MiddlewareConfiguration& rhs)
{
    return lhs.registry == rhs.registry
        && lhs.enableDomainSockets == rhs.enableDomainSockets
        && lhs.tcpNoDelay == rhs.tcpNoDelay
        && lhs.tcpQuickAck == rhs.tcpQuickAck
        && lhs.tcpReceiveBufferSize == rhs.tcpReceiveBufferSize
        && lhs.tcpSendBufferSize == rhs.tcpSendBufferSize;
}

} // namespace datatypes

// ================================================================================
//  Public API
// ================================================================================

auto ReadMiddlewareConfigurationFromYamlString(const std::string& yamlString) -> std::shared_ptr<IMiddlewareConfiguration>
{
    throw configuration_error{ "Not implemented" };
}

auto ReadMiddlewareConfigurationFromYamlFile(const std::string& yamlFilename) -> std::shared_ptr<IMiddlewareConfiguration>
{
    throw configuration_error{ "Not implemented" };
}

auto ReadMiddlewareConfigurationFromJsonString(const std::string& jsonString) -> std::shared_ptr<IMiddlewareConfiguration>
{
    throw configuration_error{ "Not implemented" };
}

auto ReadMiddlewareConfigurationFromJsonFile(const std::string& jsonFilename) -> std::shared_ptr<IMiddlewareConfiguration>
{
    throw configuration_error{ "Not implemented" };
}

auto MiddlewareConfiguration::ToYamlString() -> std::string
{
    throw configuration_error{ "Not implemented" };
}

auto MiddlewareConfiguration::ToJsonString() -> std::string
{
    throw configuration_error{ "Not implemented" };
}

} // namespace v1

} // namespace vasio
} // namespace cfg
} // namespace ib
