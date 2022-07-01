// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "ib/cfg/IParticipantConfiguration.hpp"
#include "ib/sim/fr/FlexrayDatatypes.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

#include "Configuration.hpp"
#include "Optional.hpp"

namespace ib {
namespace cfg {

inline namespace v4 {

// ================================================================================
//  Internal controller service
// ================================================================================

//! \brief Generic dummy for all internal controllers - do not make available to public API!
struct InternalController
{
    static constexpr NetworkType networkType = NetworkType::Undefined;

    std::string name;
    ib::util::Optional<std::string> network;
};

// ================================================================================
//  CAN controller service
// ================================================================================

//! \brief CAN controller service
struct CanController
{
    static constexpr NetworkType networkType = NetworkType::CAN;

    std::string name;
    ib::util::Optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  LIN controller service
// ================================================================================

//! \brief LIN controller service
struct LinController
{
    static constexpr NetworkType networkType = NetworkType::LIN;

    std::string name;
    ib::util::Optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Ethernet controller service
// ================================================================================

//! \brief Ethernet controller service
struct EthernetController
{
    static constexpr NetworkType networkType = NetworkType::Ethernet;

    std::string name;
    ib::util::Optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  FlexRay controller service
// ================================================================================

//! \brief FlexRay controller service
struct FlexrayController
{
    static constexpr NetworkType networkType = NetworkType::FlexRay;

    std::string name;
    ib::util::Optional<std::string> network;

    ib::util::Optional<sim::fr::FlexrayClusterParameters> clusterParameters;
    ib::util::Optional<sim::fr::FlexrayNodeParameters> nodeParameters;
    std::vector<sim::fr::FlexrayTxBufferConfig> txBufferConfigurations;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Data Publisher/Subscriber service
// ================================================================================

//! \brief Publisher configuration for the Data communication service
struct DataPublisher
{
    static constexpr NetworkType networkType = NetworkType::Data;

    std::string name;
    ib::util::Optional<std::string> topic;

    //! \brief History length of a DataPublisher.
    ib::util::Optional<size_t> history{ 0 };

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

//! \brief Subscriber configuration for the Data communication service
struct DataSubscriber
{
    static constexpr NetworkType networkType = NetworkType::Data;

    std::string name;
    ib::util::Optional<std::string> topic;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  RPC Client/Server service
// ================================================================================

//! \brief Server configuration for the RPC communication service
struct RpcServer
{
    static constexpr NetworkType networkType = NetworkType::RPC;

    std::string name;
    ib::util::Optional<std::string> functionName;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

//! \brief Client configuration for the RPC communication service
struct RpcClient
{
    static constexpr NetworkType networkType = NetworkType::RPC;

    std::string name;
    ib::util::Optional<std::string> functionName;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Health checking service
// ================================================================================

//! \brief Health checking service
struct HealthCheck
{
    ib::util::Optional<std::chrono::milliseconds> softResponseTimeout;
    ib::util::Optional<std::chrono::milliseconds> hardResponseTimeout;
};

// ================================================================================
//  Tracing service
// ================================================================================

//! \brief Structure that contains a participant's setup of the tracing service
struct Tracing
{
    std::vector<TraceSink> traceSinks;
    std::vector<TraceSource> traceSources;
};

// ================================================================================
//  Extensions
// ================================================================================

//! \brief Structure that contains extension settings
struct Extensions
{
    //! \brief Additional search path hints for extensions
    std::vector<std::string> searchPathHints;
};

// ================================================================================
//  VAsio Middleware
// ================================================================================

struct Middleware
{
    std::string registryUri{ "vib://localhost:8500" }; //!< Default registry URI to connect to
    int connectAttempts{ 1 }; //!<  Number of connection attempts to the registry a participant should perform.
    int tcpReceiveBufferSize{ -1 };
    int tcpSendBufferSize{ -1 };
    bool tcpNoDelay{ false }; //!< Disables Nagle's algorithm.
    bool tcpQuickAck{ false }; //!< Setting this Linux specific flag disables delayed TCP/IP acknowledgements.
    bool enableDomainSockets{ true }; //!< By default local domain socket is preferred to TCP/IP sockets.
};

// ================================================================================
//  Root
// ================================================================================

//! \brief ParticipantConfiguration is the main configuration data object for a VIB participant.
struct ParticipantConfiguration
    : public IParticipantConfiguration
{
    ParticipantConfiguration() = default;

    //virtual auto ToYamlString() -> std::string override;
    //virtual auto ToJsonString() -> std::string override;

    //! \brief Version of the JSON/YAML schema.
    std::string schemaVersion{ "1" };
    //! \brief An optional user description for documentation purposes. Currently unused.
    std::string description;
    //! \brief An optional file path.
    std::string configurationFilePath;

    //! \brief The participant name. Mandatory.
    std::string participantName;

    std::vector<CanController> canControllers;
    std::vector<LinController> linControllers;
    std::vector<EthernetController> ethernetControllers;
    std::vector<FlexrayController> flexrayControllers;

    std::vector<DataPublisher> dataPublishers;
    std::vector<DataSubscriber> dataSubscribers;

    std::vector<RpcServer> rpcServers;
    std::vector<RpcClient> rpcClients;

    Logging logging;
    HealthCheck healthCheck;
    Tracing tracing;
    Extensions extensions;
    Middleware middleware;
};

bool operator==(const CanController& lhs, const CanController& rhs);
bool operator==(const LinController& lhs, const LinController& rhs);
bool operator==(const EthernetController& lhs, const EthernetController& rhs);
bool operator==(const FlexrayController& lhs, const FlexrayController& rhs);
bool operator==(const DataPublisher& lhs, const DataPublisher& rhs);
bool operator==(const DataSubscriber& lhs, const DataSubscriber& rhs);
bool operator==(const RpcServer& lhs, const RpcServer& rhs);
bool operator==(const RpcClient& lhs, const RpcClient& rhs);
bool operator==(const HealthCheck& lhs, const HealthCheck& rhs);
bool operator==(const Tracing& lhs, const Tracing& rhs);
bool operator==(const Extensions& lhs, const Extensions& rhs);
bool operator==(const Middleware& lhs, const Middleware& rhs);
bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs);

} // namespace v4

} // namespace cfg
} // namespace ib
