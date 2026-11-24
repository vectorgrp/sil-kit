// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "silkit/util/Span.hpp"
#include "silkit/util/serdes/Serialization.hpp"

/*! \file
 * \brief Experimental: names and payloads of the SIL Kit management simulation.
 *
 * A registry with `Experimental.Management.Enable` hosts a management participant in the simulation
 * \ref SilKit::Experimental::Management::SimulationName (e.g.
 * `silkit://localhost:8500/.well-known/sil-kit/management`). It publishes registry information on the topics
 * below and serves the functions below via RPC. All payloads use the SIL Kit Adaptive Binary (SAB) serialization
 * of \ref SilKit::Util::SerDes.
 *
 * Versioning: the major version is part of every topic and function name. Minor versions only add topics
 * and functions; the payloads of an existing name do not change.
 * \ref SilKit::Experimental::Management::RegistryInfo reports the versions and features offered by the
 * management participant.
 *
 * \warning Experimental: names, payloads and behavior may change or be removed without notice.
 */

namespace SilKit {
namespace Experimental {
namespace Management {

// ================================================================================
//  Names
// ================================================================================

//! Simulation name (registry URI path) of the management simulation.
constexpr auto SimulationName() -> const char*
{
    return ".well-known/sil-kit/management";
}

//! Participant name of the management participant of the registry.
constexpr auto RegistryParticipantName() -> const char*
{
    return "sil-kit-registry";
}

//! Media type of all management topics and functions.
constexpr auto MediaType() -> const char*
{
    return "application/vnd.vector.silkit.management; protocolVersion=1";
}

constexpr auto ApiVersionMajor() -> uint32_t
{
    return 1;
}

constexpr auto ApiVersionMinor() -> uint32_t
{
    return 0;
}

namespace Registry {

//! Topic (history 1) carrying a \ref RegistryInfo.
constexpr auto InfoTopic() -> const char*
{
    return "sil-kit/management/v1/registry/info";
}

//! Topic (history 1) carrying the full \ref Simulations state on every change.
constexpr auto SimulationsTopic() -> const char*
{
    return "sil-kit/management/v1/registry/simulations";
}

//! Function without argument, returns a \ref RegistryInfo.
constexpr auto GetInfoFunction() -> const char*
{
    return "sil-kit/management/v1/registry/GetInfo";
}

//! Function without argument, returns the current \ref Simulations.
constexpr auto GetSimulationsFunction() -> const char*
{
    return "sil-kit/management/v1/registry/GetSimulations";
}

//! Function taking a \ref ParticipantConfigurationRequest, returns a \ref ParticipantConfigurationReply.
constexpr auto GetParticipantConfigurationFunction() -> const char*
{
    return "sil-kit/management/v1/registry/GetParticipantConfiguration";
}

//! Feature name reported in \ref RegistryInfo::features for the simulations topic and function.
constexpr auto FeatureSimulations() -> const char*
{
    return "simulations";
}

//! Feature name reported in \ref RegistryInfo::features for the participant configuration function.
constexpr auto FeatureParticipantConfiguration() -> const char*
{
    return "participant-configuration";
}

} // namespace Registry

/*! \brief Builds the URI of the management simulation from a registry URI.
 *
 * Any simulation path of \p registryUri is replaced, e.g. `silkit://host:8500/sim` becomes
 * `silkit://host:8500/.well-known/sil-kit/management`.
 */
inline auto MakeManagementUri(const std::string& registryUri) -> std::string
{
    const std::string schemeSeparator{"://"};
    const auto schemeEnd = registryUri.find(schemeSeparator);
    const auto authorityBegin = (schemeEnd == std::string::npos) ? 0 : schemeEnd + schemeSeparator.size();
    const auto pathBegin = registryUri.find('/', authorityBegin);
    return registryUri.substr(0, pathBegin) + "/" + SimulationName();
}

// ================================================================================
//  Payloads
// ================================================================================

struct RegistryInfo
{
    uint32_t apiVersionMajor{0};
    uint32_t apiVersionMinor{0};
    std::string silKitVersion;
    std::vector<std::string> features;
};

struct SimulationInfo
{
    std::string name;
    std::vector<std::string> participantNames;
    //! As last set by a SystemController of the simulation; empty if none was set.
    std::vector<std::string> requiredParticipantNames;
};

struct Simulations
{
    //! Incremented by the management participant on every change.
    uint64_t sequence{0};
    //! All simulations on the registry, sorted by name. The management simulation is not included.
    std::vector<SimulationInfo> simulations;
};

struct ParticipantConfigurationRequest
{
    std::string simulationName;
    std::string participantName;
};

enum class ParticipantConfigurationStatus : uint8_t
{
    Available = 0,
    //! The participant is connected, but did not set `Experimental.Management.ShareConfiguration`.
    NotShared = 1,
    UnknownParticipant = 2,
};

struct ParticipantConfigurationReply
{
    ParticipantConfigurationStatus status{ParticipantConfigurationStatus::UnknownParticipant};
    //! The participant configuration as JSON, if \ref status is \ref ParticipantConfigurationStatus::Available.
    std::string configurationJson;
};

// ================================================================================
//  Serialization
// ================================================================================

namespace Detail {

inline void SerializeStrings(SilKit::Util::SerDes::Serializer& serializer, const std::vector<std::string>& strings)
{
    serializer.BeginArray(strings.size());
    for (const auto& string : strings)
    {
        serializer.Serialize(string);
    }
    serializer.EndArray();
}

inline auto DeserializeStrings(SilKit::Util::SerDes::Deserializer& deserializer) -> std::vector<std::string>
{
    std::vector<std::string> strings(deserializer.BeginArray());
    for (auto& string : strings)
    {
        string = deserializer.Deserialize<std::string>();
    }
    deserializer.EndArray();
    return strings;
}

} // namespace Detail

inline auto Serialize(const RegistryInfo& value) -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(value.apiVersionMajor, 32);
    serializer.Serialize(value.apiVersionMinor, 32);
    serializer.Serialize(value.silKitVersion);
    Detail::SerializeStrings(serializer, value.features);
    serializer.EndStruct();
    return serializer.ReleaseBuffer();
}

inline auto Serialize(const Simulations& value) -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(value.sequence, 64);
    serializer.BeginArray(value.simulations.size());
    for (const auto& simulation : value.simulations)
    {
        serializer.BeginStruct();
        serializer.Serialize(simulation.name);
        Detail::SerializeStrings(serializer, simulation.participantNames);
        Detail::SerializeStrings(serializer, simulation.requiredParticipantNames);
        serializer.EndStruct();
    }
    serializer.EndArray();
    serializer.EndStruct();
    return serializer.ReleaseBuffer();
}

inline auto Serialize(const ParticipantConfigurationRequest& value) -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(value.simulationName);
    serializer.Serialize(value.participantName);
    serializer.EndStruct();
    return serializer.ReleaseBuffer();
}

inline auto Serialize(const ParticipantConfigurationReply& value) -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(static_cast<uint8_t>(value.status), 8);
    serializer.Serialize(value.configurationJson);
    serializer.EndStruct();
    return serializer.ReleaseBuffer();
}

template <typename T>
auto Deserialize(SilKit::Util::Span<const uint8_t> data) -> T;

template <>
inline auto Deserialize<RegistryInfo>(SilKit::Util::Span<const uint8_t> data) -> RegistryInfo
{
    SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(data)};
    RegistryInfo value;
    deserializer.BeginStruct();
    value.apiVersionMajor = deserializer.Deserialize<uint32_t>(32);
    value.apiVersionMinor = deserializer.Deserialize<uint32_t>(32);
    value.silKitVersion = deserializer.Deserialize<std::string>();
    value.features = Detail::DeserializeStrings(deserializer);
    deserializer.EndStruct();
    return value;
}

template <>
inline auto Deserialize<Simulations>(SilKit::Util::Span<const uint8_t> data) -> Simulations
{
    SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(data)};
    Simulations value;
    deserializer.BeginStruct();
    value.sequence = deserializer.Deserialize<uint64_t>(64);
    value.simulations.resize(deserializer.BeginArray());
    for (auto& simulation : value.simulations)
    {
        deserializer.BeginStruct();
        simulation.name = deserializer.Deserialize<std::string>();
        simulation.participantNames = Detail::DeserializeStrings(deserializer);
        simulation.requiredParticipantNames = Detail::DeserializeStrings(deserializer);
        deserializer.EndStruct();
    }
    deserializer.EndArray();
    deserializer.EndStruct();
    return value;
}

template <>
inline auto Deserialize<ParticipantConfigurationRequest>(SilKit::Util::Span<const uint8_t> data)
    -> ParticipantConfigurationRequest
{
    SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(data)};
    ParticipantConfigurationRequest value;
    deserializer.BeginStruct();
    value.simulationName = deserializer.Deserialize<std::string>();
    value.participantName = deserializer.Deserialize<std::string>();
    deserializer.EndStruct();
    return value;
}

template <>
inline auto Deserialize<ParticipantConfigurationReply>(SilKit::Util::Span<const uint8_t> data)
    -> ParticipantConfigurationReply
{
    SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(data)};
    ParticipantConfigurationReply value;
    deserializer.BeginStruct();
    value.status = static_cast<ParticipantConfigurationStatus>(deserializer.Deserialize<uint8_t>(8));
    value.configurationJson = deserializer.Deserialize<std::string>();
    deserializer.EndStruct();
    return value;
}

} // namespace Management
} // namespace Experimental
} // namespace SilKit
