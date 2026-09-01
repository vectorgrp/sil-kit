// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

/*! Wire-format guard for the dashboard's JSON payloads.
 *
 *  Every expected string in this file was derived mechanically from the output of the oatpp
 *  ObjectMapper that used to produce these payloads (configured as DashboardComponents did:
 *  includeNullFields on, beautifier off), by applying exactly three transformations - the three
 *  known, accepted differences between oatpp's serializer and rapidyaml's JSON emitter:
 *
 *    1. rapidyaml writes `"key": value`; oatpp wrote `"key":value`.
 *    2. rapidyaml does not escape '/'; oatpp emitted it as `\/`.
 *    3. rapidyaml emits no \uXXXX escapes, so non-ASCII stays raw UTF-8. The C0 control bytes
 *       rapidyaml cannot escape at all are replaced with U+FFFD by DashboardJsonWriter, because
 *       emitting them raw would produce invalid JSON.
 *
 *  All three are semantically transparent to a JSON parser. Any *other* difference from what the
 *  dashboard service used to receive - a reordered key, a dropped field, a string emitted as a bare
 *  number, a differently formatted double - fails a test here. That is the point of the file, so
 *  when a test below fails, do not update the expectation without establishing that the dashboard
 *  service accepts the new bytes.
 *
 *  Expectations are written as ASCII-only escapes so the file does not depend on source encoding.
 */

#include "dashboard/json/DashboardJson.hpp"

#include <limits>
#include <string>

#include "gtest/gtest.h"

namespace SilKit {
namespace Dashboard {
namespace {

auto MakeLabel(std::string key, std::string value, LabelKind kind) -> MatchingLabelDto
{
    MatchingLabelDto label{};
    label.key = std::move(key);
    label.value = std::move(value);
    label.kind = kind;
    return label;
}

auto MakeController(uint64_t id, std::string name, std::string networkName) -> BulkControllerDto
{
    BulkControllerDto controller{};
    controller.id = id;
    controller.name = std::move(name);
    controller.networkName = std::move(networkName);
    return controller;
}

// --- an empty bulk update ---------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, BulkSimulationDto_Default)
{
    EXPECT_EQ(ToJson(BulkSimulationDto{}),
              "{\"stopped\": null,\"system\": {\"statuses\": []},\"participants\": []}");
}

// --- simulation creation ----------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, SimulationCreationRequestDto_Populated)
{
    SimulationCreationRequestDto request{};
    request.started = 1758000000000ULL;
    request.configuration.connectUri = "silkit://myhost:1234";

    EXPECT_EQ(ToJson(request),
              "{\"started\": 1758000000000,\"configuration\": {\"connectUri\": \"silkit://myhost:1234\"}}");
}

TEST(Test_DashboardJsonWriter, SimulationCreationRequestDto_ExtremeValues)
{
    SimulationCreationRequestDto request{};
    request.started = std::numeric_limits<uint64_t>::max();
    request.configuration.connectUri = "";

    EXPECT_EQ(ToJson(request), "{\"started\": 18446744073709551615,\"configuration\": {\"connectUri\": \"\"}}");
}

// --- states -----------------------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, SystemStatusDto_StateIsEmittedAsItsName)
{
    SystemStatusDto status{};
    status.state = SystemState::ReadyToRun;

    EXPECT_EQ(ToJson(status), "{\"state\": \"readytorun\"}");
}

TEST(Test_DashboardJsonWriter, ParticipantStatusDto_AnEmptyReasonStaysAnEmptyStringNotNull)
{
    ParticipantStatusDto status{};
    status.state = ParticipantState::Running;
    status.enterReason = "";
    status.enterTime = 0;

    EXPECT_EQ(ToJson(status), "{\"state\": \"running\",\"enterReason\": \"\",\"enterTime\": 0}");
}

TEST(Test_DashboardJsonWriter, ParticipantStatusDto_EscapesQuotesBackslashesAndWhitespace)
{
    ParticipantStatusDto status{};
    status.state = ParticipantState::Error;
    status.enterReason = "quote:\" back:\\ nl:\n cr:\r tab:\t";
    status.enterTime = std::numeric_limits<uint64_t>::max();

    EXPECT_EQ(ToJson(status), "{\"state\": \"error\",\"enterReason\": \"quote:\\\" back:\\\\ nl:\\n cr:\\r "
                              "tab:\\t\",\"enterTime\": 18446744073709551615}");
}

// oatpp escaped non-ASCII as \uXXXX; rapidyaml passes UTF-8 through. Both decode identically.
TEST(Test_DashboardJsonWriter, ParticipantStatusDto_NonAsciiIsEmittedAsRawUtf8)
{
    ParticipantStatusDto status{};
    status.state = ParticipantState::Stopped;
    status.enterReason = "Fahrzeug-S\xc3\xbc" "d \xe2\x82\xac"; // "Fahrzeug-Sued EUR" in UTF-8
    status.enterTime = 1;

    EXPECT_EQ(ToJson(status),
              "{\"state\": \"stopped\",\"enterReason\": \"Fahrzeug-S\xc3\xbc" "d \xe2\x82\xac\",\"enterTime\": 1}");
}

// rapidyaml escapes only \b \f \n \r \t, so any other C0 byte would be emitted raw and break the
// JSON. DashboardJsonWriter substitutes U+FFFD instead.
TEST(Test_DashboardJsonWriter, ParticipantStatusDto_UnescapableControlCharactersBecomeReplacementCharacters)
{
    ParticipantStatusDto status{};
    status.state = ParticipantState::Aborting;
    status.enterReason = std::string("bell:\x07 vt:\x0b esc:\x1b");
    status.enterTime = 2;

    EXPECT_EQ(ToJson(status), "{\"state\": \"aborting\",\"enterReason\": \"bell:\xef\xbf\xbd vt:\xef\xbf\xbd "
                              "esc:\xef\xbf\xbd\",\"enterTime\": 2}");
}

// --- labels -----------------------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, MatchingLabelDto_KindIsEmittedAsItsName)
{
    EXPECT_EQ(ToJson(MakeLabel("k", "v", LabelKind::Optional)),
              "{\"key\": \"k\",\"value\": \"v\",\"kind\": \"optional\"}");
    EXPECT_EQ(ToJson(MakeLabel("k", "v", LabelKind::Mandatory)),
              "{\"key\": \"k\",\"value\": \"v\",\"kind\": \"mandatory\"}");
}

// --- string-versus-number typing --------------------------------------------------------------

/*! rapidyaml emits a scalar unquoted unless it is told otherwise, and treats anything number-like
 *  as plain, so without the writer's forced quoting these string fields would silently turn into
 *  JSON numbers and change the payload's types.
 */
TEST(Test_DashboardJsonWriter, StringFields_ThatLookLikeNumbers_StayQuoted)
{
    EXPECT_EQ(ToJson(MakeController(0, "12345", "0")),
              "{\"id\": 0,\"name\": \"12345\",\"networkName\": \"0\"}");
}

TEST(Test_DashboardJsonWriter, StringFields_ThatLookLikeOtherJsonLiterals_StayQuoted)
{
    BulkDataServiceDto service{};
    service.id = 7;
    service.name = "3.14";
    service.networkName = "1_000";
    service.spec.topic = "1e5";
    service.spec.mediaType = "007";
    service.spec.labels.push_back(MakeLabel("0x10", "-0", LabelKind::Optional));
    service.spec.labels.push_back(MakeLabel("true", "null", LabelKind::Mandatory));

    EXPECT_EQ(ToJson(service),
              "{\"id\": 7,\"name\": \"3.14\",\"networkName\": \"1_000\",\"spec\": {\"topic\": \"1e5\",\"mediaType\": "
              "\"007\",\"labels\": [{\"key\": \"0x10\",\"value\": \"-0\",\"kind\": \"optional\"},{\"key\": "
              "\"true\",\"value\": \"null\",\"kind\": \"mandatory\"}]}}");
}

TEST(Test_DashboardJsonWriter, BulkServiceInternalDto_Populated)
{
    BulkServiceInternalDto service{};
    service.id = 9;
    service.name = "n";
    service.networkName = "net";
    service.parentId = 10;

    EXPECT_EQ(ToJson(service), "{\"id\": 9,\"name\": \"n\",\"networkName\": \"net\",\"parentId\": 10}");
}

// --- the full bulk update ---------------------------------------------------------------------

/*! Covers the whole nested structure, and in particular pins BulkParticipantDto's sixteen keys in
 *  their declared order; the dashboard service is sensitive to the payload shape.
 */
TEST(Test_DashboardJsonWriter, BulkSimulationDto_FullyPopulated)
{
    BulkSystemDto system{};
    for (const auto state : {SystemState::ServicesCreated, SystemState::Running, SystemState::Shutdown})
    {
        SystemStatusDto status{};
        status.state = state;
        system.statuses.push_back(status);
    }

    BulkParticipantDto participant{};
    participant.name = "P1";
    {
        ParticipantStatusDto status{};
        status.state = ParticipantState::Running;
        status.enterReason = "ok";
        status.enterTime = 42;
        participant.statuses.push_back(status);
    }
    participant.canControllers.push_back(MakeController(1, "can1", "CAN1"));
    participant.ethernetControllers.push_back(MakeController(2, "eth1", "ETH1"));
    participant.flexrayControllers.push_back(MakeController(3, "fr1", "FR1"));
    participant.linControllers.push_back(MakeController(4, "lin1", "LIN1"));
    {
        BulkDataServiceDto publisher{};
        publisher.id = 5;
        publisher.name = "pub";
        publisher.networkName = "N";
        publisher.spec.topic = "t";
        publisher.spec.mediaType = "m";
        publisher.spec.labels.push_back(MakeLabel("lk", "lv", LabelKind::Mandatory));
        participant.dataPublishers.push_back(publisher);
    }
    {
        BulkServiceInternalDto internal{};
        internal.id = 6;
        internal.name = "sub_int";
        internal.networkName = "N";
        internal.parentId = 5;
        participant.dataSubscriberInternals.push_back(internal);
    }
    {
        BulkRpcServiceDto client{};
        client.id = 11;
        client.name = "client";
        client.networkName = "N";
        client.spec.functionName = "f";
        client.spec.mediaType = "m";
        participant.rpcClients.push_back(client);
    }
    participant.canNetworks.push_back("CAN1");
    participant.ethernetNetworks.push_back("ETH1");
    participant.flexrayNetworks.push_back("FR1");
    participant.linNetworks.push_back("LIN1");

    BulkSimulationDto bulk{};
    bulk.stopped = std::numeric_limits<int64_t>::min();
    bulk.system = system;
    bulk.participants.push_back(participant);

    EXPECT_EQ(
        ToJson(bulk),
        "{\"stopped\": -9223372036854775808,\"system\": {\"statuses\": [{\"state\": \"servicescreated\"},{\"state\": "
        "\"running\"},{\"state\": \"shutdown\"}]},\"participants\": [{\"name\": \"P1\",\"statuses\": [{\"state\": "
        "\"running\",\"enterReason\": \"ok\",\"enterTime\": 42}],\"canControllers\": [{\"id\": 1,\"name\": "
        "\"can1\",\"networkName\": \"CAN1\"}],\"ethernetControllers\": [{\"id\": 2,\"name\": \"eth1\",\"networkName\": "
        "\"ETH1\"}],\"flexrayControllers\": [{\"id\": 3,\"name\": \"fr1\",\"networkName\": "
        "\"FR1\"}],\"linControllers\": [{\"id\": 4,\"name\": \"lin1\",\"networkName\": \"LIN1\"}],\"dataPublishers\": "
        "[{\"id\": 5,\"name\": \"pub\",\"networkName\": \"N\",\"spec\": {\"topic\": \"t\",\"mediaType\": "
        "\"m\",\"labels\": [{\"key\": \"lk\",\"value\": \"lv\",\"kind\": "
        "\"mandatory\"}]}}],\"dataSubscribers\": [],\"dataSubscriberInternals\": [{\"id\": 6,\"name\": "
        "\"sub_int\",\"networkName\": \"N\",\"parentId\": 5}],\"rpcClients\": [{\"id\": 11,\"name\": "
        "\"client\",\"networkName\": \"N\",\"spec\": {\"functionName\": \"f\",\"mediaType\": \"m\",\"labels\": "
        "[]}}],\"rpcServers\": [],\"rpcServerInternals\": [],\"canNetworks\": [\"CAN1\"],\"ethernetNetworks\": "
        "[\"ETH1\"],\"flexrayNetworks\": [\"FR1\"],\"linNetworks\": [\"LIN1\"]}]}");
}

// --- metrics ----------------------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, AttributeDataDto_EmitsBaseFieldsBeforeTheValue)
{
    AttributeDataDto attribute{};
    attribute.ts = 1700000000000LL;
    attribute.pn = "P1";
    attribute.mn = {"a", "b"};
    attribute.mv = "plain";

    EXPECT_EQ(ToJson(attribute), "{\"ts\": 1700000000000,\"pn\": \"P1\",\"mn\": [\"a\",\"b\"],\"mv\": \"plain\"}");
}

// A STRING_LIST metric carries its list as an opaque string, which must stay a JSON string.
TEST(Test_DashboardJsonWriter, AttributeDataDto_AStringListValueStaysANestedString)
{
    AttributeDataDto attribute{};
    attribute.ts = 1;
    attribute.pn = "P1";
    attribute.mn = {"names"};
    attribute.mv = "[\"a\",\"b\"]";

    EXPECT_EQ(ToJson(attribute),
              "{\"ts\": 1,\"pn\": \"P1\",\"mn\": [\"names\"],\"mv\": \"[\\\"a\\\",\\\"b\\\"]\"}");
}

TEST(Test_DashboardJsonWriter, CounterDataDto_HandlesTheFullInt64Range)
{
    CounterDataDto counter{};
    counter.ts = 2;
    counter.pn = "P1";
    counter.mn = {"c"};
    counter.mv = std::numeric_limits<int64_t>::min();

    EXPECT_EQ(ToJson(counter), "{\"ts\": 2,\"pn\": \"P1\",\"mn\": [\"c\"],\"mv\": -9223372036854775808}");
}

TEST(Test_DashboardJsonWriter, StatisticDataDto_FormatsDoublesTheWayOatppDid)
{
    StatisticDataDto statistic{};
    statistic.ts = 3;
    statistic.pn = "P1";
    statistic.mn = {"s"};
    statistic.mv = {1.0, 2.5, 0.1, 100.0};

    EXPECT_EQ(ToJson(statistic), "{\"ts\": 3,\"pn\": \"P1\",\"mn\": [\"s\"],\"mv\": [1,2.5,0.1,100]}");
}

/*! oatpp formatted doubles with "%.16g", which truncates a shortest-round-trip 17-digit double.
 *  The writer keeps that format, so the values the dashboard receives do not change.
 */
TEST(Test_DashboardJsonWriter, StatisticDataDto_KeepsOatppsSixteenSignificantDigits)
{
    StatisticDataDto statistic{};
    statistic.ts = 4;
    statistic.pn = "P1";
    statistic.mn = {"s17"};
    statistic.mv = {0.1234567890123456789, 1e-300, 1.7976931348623157e308};

    EXPECT_EQ(ToJson(statistic), "{\"ts\": 4,\"pn\": \"P1\",\"mn\": [\"s17\"],\"mv\": "
                                 "[0.1234567890123457,1e-300,1.797693134862316e+308]}");
}

TEST(Test_DashboardJsonWriter, MetricsUpdateDto_Empty)
{
    EXPECT_EQ(ToJson(MetricsUpdateDto{}), "{\"attributes\": [],\"counters\": [],\"statistics\": []}");
}

TEST(Test_DashboardJsonWriter, MetricsUpdateDto_OneOfEachKind)
{
    AttributeDataDto attribute{};
    attribute.ts = 1700000000000LL;
    attribute.pn = "P1";
    attribute.mn = {"a", "b"};
    attribute.mv = "plain";

    CounterDataDto counter{};
    counter.ts = 2;
    counter.pn = "P1";
    counter.mn = {"c"};
    counter.mv = std::numeric_limits<int64_t>::min();

    StatisticDataDto statistic{};
    statistic.ts = 3;
    statistic.pn = "P1";
    statistic.mn = {"s"};
    statistic.mv = {1.0, 2.5, 0.1, 100.0};

    MetricsUpdateDto update{};
    update.attributes.push_back(attribute);
    update.counters.push_back(counter);
    update.statistics.push_back(statistic);

    EXPECT_EQ(ToJson(update),
              "{\"attributes\": [{\"ts\": 1700000000000,\"pn\": \"P1\",\"mn\": [\"a\",\"b\"],\"mv\": "
              "\"plain\"}],\"counters\": [{\"ts\": 2,\"pn\": \"P1\",\"mn\": [\"c\"],\"mv\": "
              "-9223372036854775808}],\"statistics\": [{\"ts\": 3,\"pn\": \"P1\",\"mn\": [\"s\"],\"mv\": "
              "[1,2.5,0.1,100]}]}");
}

// --- response parsing -------------------------------------------------------------------------

TEST(Test_DashboardJsonWriter, ParseSimulationCreationResponse_ReadsTheId)
{
    EXPECT_EQ(ParseSimulationCreationResponse(R"({"id":42})"), 42u);
}

TEST(Test_DashboardJsonWriter, ParseSimulationCreationResponse_HandlesTheFullUint64Range)
{
    EXPECT_EQ(ParseSimulationCreationResponse(R"({"id":18446744073709551615})"),
              std::numeric_limits<uint64_t>::max());
}

/*! oatpp rejected unknown fields, and the resulting exception propagated out of the dashboard's
 *  worker thread and killed it. Tolerating them is a deliberate behaviour change.
 */
TEST(Test_DashboardJsonWriter, ParseSimulationCreationResponse_IgnoresUnknownFields)
{
    EXPECT_EQ(ParseSimulationCreationResponse(R"({"id":7,"extra":"whatever","nested":{"a":1}})"), 7u);
}

TEST(Test_DashboardJsonWriter, ParseSimulationCreationResponse_ReturnsNulloptForUnusableBodies)
{
    EXPECT_FALSE(ParseSimulationCreationResponse("").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse("not json at all").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse("[1,2,3]").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse(R"({"other":1})").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse(R"({"id":"not a number"})").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse(R"({"id":null})").has_value());
    EXPECT_FALSE(ParseSimulationCreationResponse(R"({"id":99999999999999999999999})").has_value());
}

} // namespace
} // namespace Dashboard
} // namespace SilKit
