// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <string>

#include "silkit/services/all.hpp"

#include "SimTestHarness.hpp"
#include "ITestLogFiles.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Tests;

using SilKit::IntegrationTests::FindLogFiles;
using SilKit::IntegrationTests::ReadTextFile;

const std::string sender1InfoMessage = "remote-log-from-sender-1";
const std::string sender2InfoMessage = "remote-log-from-sender-2";
const std::string sender1WarnMessage = "remote-warning-from-sender-1";
const std::string receiverOwnMessage = "local-log-from-receiver";

// Runs Sender1, Sender2 and Receiver for a few simulation steps. In their first step, the senders log an info message
// and Sender1 additionally a warning; the receiver logs a message of its own. Returns the receiver's file sink contents.
auto RunRemoteLoggingSimulation(const std::string& senderConfig, const std::string& receiverLoggingSection)
    -> std::string
{
    const auto receiverLogName = SilKit::IntegrationTests::MakeUniqueLogName("itest_remote_logging");
    const auto filePrefix = receiverLogName + "_Receiver_";
    SilKit::IntegrationTests::ScopedLogFiles cleanup{filePrefix};

    const auto receiverConfig = R"(
Logging:
)" + receiverLoggingSection + R"(
  FlushLevel: Trace
  Sinks:
    - Type: File
      Level: Trace
      LogName: )" + receiverLogName + "\n";

    SimTestHarnessArgs testHarnessArgs;
    testHarnessArgs.syncParticipantNames = {"Sender1", "Sender2", "Receiver"};
    testHarnessArgs.deferParticipantCreation = true;

    SimTestHarness testHarness{testHarnessArgs};

    auto* sender1 = testHarness.GetParticipant("Sender1", senderConfig);
    auto* sender2 = testHarness.GetParticipant("Sender2", senderConfig);
    auto* receiver = testHarness.GetParticipant("Receiver", receiverConfig);

    auto* sender1Lifecycle = sender1->GetOrCreateLifecycleService();
    auto* sender1TimeSync = sender1->GetOrCreateTimeSyncService();
    auto* sender2TimeSync = sender2->GetOrCreateTimeSyncService();
    auto* receiverTimeSync = receiver->GetOrCreateTimeSyncService();

    auto* sender1Logger = sender1->GetLogger();
    auto* sender2Logger = sender2->GetLogger();
    auto* receiverLogger = receiver->GetLogger();

    bool sender1Logged{false};
    bool sender2Logged{false};
    bool receiverLogged{false};

    sender1TimeSync->SetSimulationStepHandler([&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (!sender1Logged)
        {
            sender1Logged = true;
            sender1Logger->Info(sender1InfoMessage);
            sender1Logger->Warn(sender1WarnMessage);
        }

        if (now >= 5ms)
        {
            sender1Lifecycle->Stop("remote logging test done");
        }
    },
        1ms);

    sender2TimeSync->SetSimulationStepHandler([&](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds) {
        if (!sender2Logged)
        {
            sender2Logged = true;
            sender2Logger->Info(sender2InfoMessage);
        }
    },
        1ms);

    receiverTimeSync->SetSimulationStepHandler([&](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds) {
        if (!receiverLogged)
        {
            receiverLogged = true;
            receiverLogger->Info(receiverOwnMessage);
        }
    },
        1ms);

    EXPECT_TRUE(testHarness.Run(5s));
    testHarness.ResetParticipants();

    const auto logFiles = FindLogFiles(filePrefix);
    EXPECT_EQ(logFiles.size(), 1u) << "Expected exactly one receiver log file with prefix " << filePrefix;
    return logFiles.empty() ? std::string{} : ReadTextFile(logFiles.front());
}

const std::string remoteTraceSenderConfig = R"(
Logging:
  Sinks:
    - Type: Remote
      Level: Trace
)";

TEST(ITest_RemoteLogging, test_remote_logging_two_senders_one_receiver)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "  LogFromRemotes: true");

    EXPECT_THAT(logContent, testing::HasSubstr(sender1InfoMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(sender1WarnMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(sender2InfoMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

// QA test case 2.8 "Logger Configuration - Remote Logging", second part (see SILKIT-1338): a participant that does not
// set LogFromRemotes must not receive the log messages of other participants.
TEST(ITest_RemoteLogging, test_log_from_remotes_false_ignores_remote_messages)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "  LogFromRemotes: false");

    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1InfoMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1WarnMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender2InfoMessage)));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

TEST(ITest_RemoteLogging, test_log_from_remotes_defaults_to_false)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "");

    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1InfoMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender2InfoMessage)));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

// The level of the sender's Remote sink decides which messages are sent at all.
TEST(ITest_RemoteLogging, test_remote_sink_level_filters_on_the_sender_side)
{
    const auto senderConfig = R"(
Logging:
  Sinks:
    - Type: Remote
      Level: Warn
)";
    const auto logContent = RunRemoteLoggingSimulation(senderConfig, "  LogFromRemotes: true");

    EXPECT_THAT(logContent, testing::HasSubstr(sender1WarnMessage));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1InfoMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender2InfoMessage)));
}

} // namespace
