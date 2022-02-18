// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>

#include "ib/cfg/vasio/IMiddlewareConfiguration.hpp"
#include "ib/IbMacros.hpp"

#include "Configuration.hpp"
#include "Optional.hpp"

namespace ib {
namespace cfg {

// ================================================================================
//  Middleware Configuration of VAsio
// ================================================================================
namespace vasio {

inline namespace v1 {

namespace datatypes {

struct Registry
{
    std::string hostname{"localhost"};
    uint16_t port{8500};
    ib::cfg::v1::datatypes::Logging logging;
    int connectAttempts{1}; //!<  Number of connection attempts to the registry a participant should perform.
};

struct MiddlewareConfiguration
{
    //! \brief Version of the JSON/YAML schema, when loaded from a JSON/YAML file.
    ib::util::Optional<std::string> schemaVersion{ "1" };
    //! \brief An optional user description for documentation purposes. Currently unused.
    ib::util::Optional<std::string> description;
    //! \brief An optional file path, when loaded from a JSON/YAML file.
    ib::util::Optional<std::string> configurationFilePath;

    Registry registry;
    int tcpReceiveBufferSize{ -1 };
    int tcpSendBufferSize{ -1 };
    bool tcpNoDelay{ false }; //!< Disables Nagle's algorithm.
    bool tcpQuickAck{ false }; //!< Setting this Linux specific flag disables delayed TCP/IP acknowledgments.
    bool enableDomainSockets{ true }; //!< By default local domain socket is preferred to TCP/IP sockets.
};

bool operator==(const Registry& lhs, const Registry& rhs);
bool operator==(const MiddlewareConfiguration& lhs, const MiddlewareConfiguration& rhs);

} // namespace datatypes

class MiddlewareConfiguration 
    : public IMiddlewareConfiguration
{
public:
    MiddlewareConfiguration(datatypes::MiddlewareConfiguration&& data)
        : _data(std::move(data))
    {
    }

    virtual auto ToYamlString() -> std::string override;
    virtual auto ToJsonString() -> std::string override;

public:
    datatypes::MiddlewareConfiguration _data;
};

inline auto CreateDummyIMiddlewareConfiguration() -> std::shared_ptr<IMiddlewareConfiguration>
{
    ib::cfg::vasio::v1::datatypes::MiddlewareConfiguration configDt;
    auto configPtr = std::make_shared<ib::cfg::vasio::MiddlewareConfiguration>(std::move(configDt));
    return std::move(configPtr);
}

inline auto CreateDummyMiddlewareConfiguration() -> std::shared_ptr<MiddlewareConfiguration>
{
    return std::make_shared<ib::cfg::vasio::MiddlewareConfiguration>(ib::cfg::vasio::v1::datatypes::MiddlewareConfiguration{});
}

} // inline namespace v1

} // namespace vasio
} // namespace cfg
} // namespace ib
