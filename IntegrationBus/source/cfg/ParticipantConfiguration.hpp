// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "ib/cfg/IParticipantConfiguration.hpp"
#include "ib/sim/fr/FrDatatypes.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "ib/sim/data/DataMessageDatatypes.hpp"

#include "Configuration.hpp"
#include "Optional.hpp"

namespace ib {
namespace cfg {

inline namespace v1 {

namespace datatypes {

// ================================================================================
//  CAN controller service
// ================================================================================

//! \brief CAN controller service
struct CanController
{
    static constexpr NetworkType networkType = NetworkType::CAN;

    std::string name;
    std::string network;

    std::vector<std::string> useTraceSinks;
    ib::util::Optional<Replay> replay;
};

// ================================================================================
//  LIN controller service
// ================================================================================

//! \brief LIN controller service
struct LinController
{
    static constexpr NetworkType networkType = NetworkType::LIN;

    std::string name;
    std::string network;

    std::vector<std::string> useTraceSinks;
    ib::util::Optional<Replay> replay;
};

// ================================================================================
//  Ethernet controller service
// ================================================================================


//! \brief Ethernet controller service
struct EthernetController
{
    static constexpr NetworkType networkType = NetworkType::Ethernet;

    std::string name;
    std::string network;

    typedef std::array<uint8_t, 6> MacAddress;
    ib::util::Optional<MacAddress> macAddress{};

    std::vector<std::string> useTraceSinks;
    ib::util::Optional<Replay> replay;
};

// ================================================================================
//  FlexRay controller service
// ================================================================================

//! \brief FlexRay controller service
struct FlexRayController
{
    static constexpr NetworkType networkType = NetworkType::FlexRay;

    std::string name;
    std::string network;

    ib::util::Optional<sim::fr::ClusterParameters> clusterParameters{};
    ib::util::Optional<sim::fr::NodeParameters> nodeParameters{};
    std::vector<sim::fr::TxBufferConfig> txBufferConfigs;

    std::vector<std::string> useTraceSinks;
    ib::util::Optional<Replay> replay;
};

// ================================================================================
//  Data Publisher/Subscriber service
// ================================================================================

/*! \brief Configuration structure to setup ib::sim::Data::IDataPublisher
 * and ib::sim::Data::IDataSubscriber
 *
 * The DataPort::protocolType and DataPort::definitionUri can only be
 * configured at the Publisher, but the configured values are made available at
 * connected Subscribers as well.
 */
struct DataPort
{
    static constexpr NetworkType networkType = NetworkType::DataMessage;

    std::string name;
    std::string network;

    //! \brief De/serialization format bound to a DataPublisher/DataSubscriber.
    ib::util::Optional<sim::data::DataExchangeFormat> dataExchangeFormat;

    //! \brief History length of a DataPublisher.
    ib::util::Optional<size_t> history = 0;

    ib::util::Optional<std::string> pubUUID{ "" };

    std::vector<std::string> useTraceSinks;
    ib::util::Optional<Replay> replay;
};

// ================================================================================
//  Heath checking service
// ================================================================================

//! \brief Health checking service
struct HealthCheck
{
    ib::util::Optional<std::chrono::milliseconds> softResponseTimeout = (std::chrono::milliseconds::max)();
    ib::util::Optional<std::chrono::milliseconds> hardResponseTimeout = (std::chrono::milliseconds::max)();
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
//  Root
// ================================================================================

//! \brief ParticipantConfiguration is the main configuration data object for a VIB participant.
struct ParticipantConfiguration
{
    //! \brief Version of the JSON/YAML schema, when loaded from a JSON/YAML file.
    ib::util::Optional<std::string> schemaVersion{ "1" };
    //! \brief An optional user description for documentation purposes. Currently unused.
    ib::util::Optional<std::string> description;
    //! \brief An optional file path, when loaded from a JSON/YAML file.
    ib::util::Optional<std::string> configurationFilePath;

    //! \brief The participant name. Mandatory.
    ib::util::Optional<std::string> participantName;

    std::vector<CanController> canControllers;
    std::vector<LinController> linControllers;
    std::vector<EthernetController> ethernetControllers;
    std::vector<FlexRayController> flexRayControllers;

    std::vector<DataPort> dataPublishers;
    std::vector<DataPort> dataSubscribers;

    ib::util::Optional<Logging> logging;
    ib::util::Optional<HealthCheck> healthCheck;
    ib::util::Optional<Tracing> tracing;
    ib::util::Optional<Extensions> extensions;
};

bool operator==(const CanController& lhs, const CanController& rhs);
bool operator==(const LinController& lhs, const LinController& rhs);
bool operator==(const EthernetController& lhs, const EthernetController& rhs);
bool operator==(const FlexRayController& lhs, const FlexRayController& rhs);
bool operator==(const DataPort& lhs, const DataPort& rhs);
bool operator==(const HealthCheck& lhs, const HealthCheck& rhs);
bool operator==(const Tracing& lhs, const Tracing& rhs);
bool operator==(const Extensions& lhs, const Extensions& rhs);
bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs);

// Note: For better maintainability we do not overload operator<< for std::array.
std::ostream& to_ostream(std::ostream& out, const std::array<uint8_t, 6>& macAddress);
std::istream& from_istream(std::istream& in, std::array<uint8_t, 6>& macAddress);

} // namespace datatypes

class ParticipantConfiguration 
    : public IParticipantConfiguration
{
public:
    ParticipantConfiguration(datatypes::ParticipantConfiguration&& data)
        : _data(std::move(data))
    {
    }

    //virtual auto ToYamlString() -> std::string override;
    //virtual auto ToJsonString() -> std::string override;

public:
    datatypes::ParticipantConfiguration _data;
};

inline auto CreateDummyConfiguration() -> std::shared_ptr<IParticipantConfiguration>
{
    ib::cfg::v1::datatypes::ParticipantConfiguration configDt;
    auto configPtr = std::make_shared<ib::cfg::ParticipantConfiguration>(std::move(configDt));
    return std::move(configPtr);
}

} // namespace v1

} // namespace cfg
} // namespace ib
