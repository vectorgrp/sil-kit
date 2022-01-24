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
    ib::util::Optional<std::string> hostname{"localhost"};
    ib::util::Optional<uint16_t> port{8500};
    ib::util::Optional<::ib::cfg::datatypes::Logging> logging;
    ib::util::Optional<int> connectAttempts{1}; //!<  Number of connection attempts to the registry a participant should perform.
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
    bool tcpQuickAck{ false }; //!< Setting this Linux specific flag disables delayed TCP/IP acknowledgements.
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

} // inline namespace v1

} // namespace vasio
} // namespace cfg
} // namespace ib
