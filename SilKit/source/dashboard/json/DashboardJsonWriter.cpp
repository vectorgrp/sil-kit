// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/json/DashboardJsonWriter.hpp"

#include <string>

namespace SilKit {
namespace Dashboard {

namespace {

//! U+FFFD REPLACEMENT CHARACTER, as UTF-8.
constexpr std::string_view kReplacementCharacter = "\xEF\xBF\xBD";

/*! True for the control bytes ryml's JSON emitter would pass through unescaped.
 *
 *  ryml escapes only " \ \b \f \n \r \t. Any other C0 byte would be emitted raw, which is invalid
 *  JSON, so those are replaced instead. Reachable only through free-text fields such as
 *  ParticipantStatus::enterReason and supplemental-data-derived label and topic values.
 */
auto NeedsReplacement(unsigned char c) -> bool
{
    switch (c)
    {
    case '\b':
    case '\t':
    case '\n':
    case '\f':
    case '\r':
        return false;
    default:
        return c < 0x20;
    }
}

} // namespace

void DashboardJsonWriter::WriteQuoted(std::string_view value)
{
    const auto needsSanitizing = [value] {
        for (const char c : value)
        {
            if (NeedsReplacement(static_cast<unsigned char>(c)))
            {
                return true;
            }
        }
        return false;
    }();

    // ryml copies the scalar into the tree arena, so a local buffer is safe here.
    std::string sanitized;
    if (needsSanitizing)
    {
        sanitized.reserve(value.size());
        for (const char c : value)
        {
            if (NeedsReplacement(static_cast<unsigned char>(c)))
            {
                sanitized += kReplacementCharacter;
            }
            else
            {
                sanitized += c;
            }
        }
        value = sanitized;
    }

    node << ryml::csubstr{value.data(), value.size()};
    // Without this a numeric-looking string would be emitted as a bare JSON number, and an empty
    // string would be emitted as null.
    node.set_val_style(ryml::VALQUO);
}

void DashboardJsonWriter::Write(const SimulationConfigurationDto& obj)
{
    MakeMap();
    WriteKeyValue("connectUri", obj.connectUri);
}

void DashboardJsonWriter::Write(const SimulationCreationRequestDto& obj)
{
    MakeMap();
    WriteKeyValue("started", obj.started);
    WriteKeyValue("configuration", obj.configuration);
}

void DashboardJsonWriter::Write(const SystemStatusDto& obj)
{
    MakeMap();
    WriteKeyValue("state", obj.state);
}

void DashboardJsonWriter::Write(const ParticipantStatusDto& obj)
{
    MakeMap();
    WriteKeyValue("state", obj.state);
    WriteKeyValue("enterReason", obj.enterReason);
    WriteKeyValue("enterTime", obj.enterTime);
}

void DashboardJsonWriter::Write(const MatchingLabelDto& obj)
{
    MakeMap();
    WriteKeyValue("key", obj.key);
    WriteKeyValue("value", obj.value);
    WriteKeyValue("kind", obj.kind);
}

void DashboardJsonWriter::Write(const DataSpecDto& obj)
{
    MakeMap();
    WriteKeyValue("topic", obj.topic);
    WriteKeyValue("mediaType", obj.mediaType);
    WriteKeyValue("labels", obj.labels);
}

void DashboardJsonWriter::Write(const RpcSpecDto& obj)
{
    MakeMap();
    WriteKeyValue("functionName", obj.functionName);
    WriteKeyValue("mediaType", obj.mediaType);
    WriteKeyValue("labels", obj.labels);
}

void DashboardJsonWriter::Write(const BulkSystemDto& obj)
{
    MakeMap();
    WriteKeyValue("statuses", obj.statuses);
}

void DashboardJsonWriter::Write(const BulkControllerDto& obj)
{
    MakeMap();
    WriteKeyValue("id", obj.id);
    WriteKeyValue("name", obj.name);
    WriteKeyValue("networkName", obj.networkName);
}

void DashboardJsonWriter::Write(const BulkDataServiceDto& obj)
{
    MakeMap();
    WriteKeyValue("id", obj.id);
    WriteKeyValue("name", obj.name);
    WriteKeyValue("networkName", obj.networkName);
    WriteKeyValue("spec", obj.spec);
}

void DashboardJsonWriter::Write(const BulkRpcServiceDto& obj)
{
    MakeMap();
    WriteKeyValue("id", obj.id);
    WriteKeyValue("name", obj.name);
    WriteKeyValue("networkName", obj.networkName);
    WriteKeyValue("spec", obj.spec);
}

void DashboardJsonWriter::Write(const BulkServiceInternalDto& obj)
{
    MakeMap();
    WriteKeyValue("id", obj.id);
    WriteKeyValue("name", obj.name);
    WriteKeyValue("networkName", obj.networkName);
    WriteKeyValue("parentId", obj.parentId);
}

void DashboardJsonWriter::Write(const BulkParticipantDto& obj)
{
    MakeMap();
    WriteKeyValue("name", obj.name);
    WriteKeyValue("statuses", obj.statuses);
    WriteKeyValue("canControllers", obj.canControllers);
    WriteKeyValue("ethernetControllers", obj.ethernetControllers);
    WriteKeyValue("flexrayControllers", obj.flexrayControllers);
    WriteKeyValue("linControllers", obj.linControllers);
    WriteKeyValue("dataPublishers", obj.dataPublishers);
    WriteKeyValue("dataSubscribers", obj.dataSubscribers);
    WriteKeyValue("dataSubscriberInternals", obj.dataSubscriberInternals);
    WriteKeyValue("rpcClients", obj.rpcClients);
    WriteKeyValue("rpcServers", obj.rpcServers);
    WriteKeyValue("rpcServerInternals", obj.rpcServerInternals);
    WriteKeyValue("canNetworks", obj.canNetworks);
    WriteKeyValue("ethernetNetworks", obj.ethernetNetworks);
    WriteKeyValue("flexrayNetworks", obj.flexrayNetworks);
    WriteKeyValue("linNetworks", obj.linNetworks);
}

void DashboardJsonWriter::Write(const BulkSimulationDto& obj)
{
    MakeMap();
    WriteKeyValueOrNull("stopped", obj.stopped);
    WriteKeyValue("system", obj.system);
    WriteKeyValue("participants", obj.participants);
}

void DashboardJsonWriter::Write(const MetricsUpdateDto& obj)
{
    MakeMap();
    WriteKeyValue("attributes", obj.attributes);
    WriteKeyValue("counters", obj.counters);
    WriteKeyValue("statistics", obj.statistics);
}

} // namespace Dashboard
} // namespace SilKit
