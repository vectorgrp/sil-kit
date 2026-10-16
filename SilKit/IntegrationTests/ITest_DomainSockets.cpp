// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Participants on the same host connect via local domain sockets by default, and via TCP if domain sockets are
// disabled.

#include <string>

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "IntegrationTestUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using IntegrationTestUtils::MakeUniqueLogName;
using IntegrationTestUtils::ScopedLogFiles;

auto MakeConfig(const std::string& logName, const std::string& middleware) -> std::string
{
    return R"(
Logging:
  FlushLevel: Trace
  Sinks:
  - Type: File
    Level: Debug
    LogName: )" + logName + "\n" + middleware;
}

// Connects two participants to a registry and returns the log of the first one.
auto ConnectTwoParticipants(const std::string& middleware) -> std::string
{
    ScopedLogFiles logFiles{MakeUniqueLogName("ITest_DomainSockets")};

    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");
    {
        const auto config = SilKit::Config::ParticipantConfigurationFromString(MakeConfig(logFiles.Prefix(), middleware));
        auto first = SilKit::CreateParticipant(config, "First", registryUri);
        // The second participant connects to the first one, which logs the incoming connection.
        auto second = SilKit::CreateParticipant(config, "Second", registryUri);
    }
    return logFiles.ReadAll();
}

TEST(ITest_DomainSockets, participants_connect_via_domain_sockets_by_default)
{
    const auto log = ConnectTwoParticipants("");

    EXPECT_THAT(log, testing::HasSubstr("Connected to registry at 'local://"))
        << "The connection to a registry on the same host must use local domain sockets";
    EXPECT_THAT(log, testing::Not(testing::HasSubstr("Connected to registry at 'tcp://")));
    EXPECT_THAT(log, testing::HasSubstr("New connection from [local=local://"))
        << "The connection between participants on the same host must use local domain sockets";
}

TEST(ITest_DomainSockets, participants_connect_via_tcp_if_domain_sockets_are_disabled)
{
    const auto log = ConnectTwoParticipants(R"(
Middleware:
  EnableDomainSockets: false
)");

    // The registry still offers its local:// acceptor, which shows up in the log, but must not be used.
    EXPECT_THAT(log, testing::HasSubstr("Connected to registry at 'tcp://"));
    EXPECT_THAT(log, testing::Not(testing::HasSubstr("Connected to registry at 'local://")));
    EXPECT_THAT(log, testing::HasSubstr("New connection from [local=tcp://"));
    EXPECT_THAT(log, testing::Not(testing::HasSubstr("New connection from [local=local://")));
}

} // namespace
