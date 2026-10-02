// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "IntegrationTestUtils.hpp"
#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace {

namespace fs = std::filesystem;

using namespace std::chrono_literals;
using namespace SilKit::Services::Logging;

using IntegrationTestUtils::FindLogFiles;
using IntegrationTestUtils::LogFilePrefix;

const std::string participantName{"LoggingParticipant"};
const std::string simpleLogName{"ITest_Logging_Simple"};
const std::string jsonLogName{"ITest_Logging_Json"};

// The demos render bus events into their log messages, and the SIL Kit stream operators use braces:
// "Received Flexray::FlexraySymbolTransmitEvent{pattern=Wus, channel=A @ 12.796ms}". Such a message must
// reach the sinks verbatim. If it is handed on as a fmt format string instead, '{pattern=' is parsed as a
// replacement field and fmt::format throws - which used to surface as a SilKitError inside the user's
// event handler.
auto MakeBracedMessage() -> std::string
{
    SilKit::Services::Flexray::FlexraySymbolTransmitEvent symbol{};
    symbol.timestamp = 12796us;
    symbol.channel = SilKit::Services::Flexray::FlexrayChannel::A;
    symbol.pattern = SilKit::Services::Flexray::FlexraySymbolPattern::Wus;

    std::stringstream ss;
    ss << "Received " << symbol;
    return ss.str();
}

auto MakeParticipantConfiguration() -> const std::string
{
    std::string config = R"(
Logging:
    FlushLevel: Trace
    Sinks:
    - Type: File
      Level: Trace
      Format: Simple
      LogName: ITest_Logging_Simple
    - Type: File
      Level: Trace
      Format: Json
      LogName: ITest_Logging_Json
    )";
    return config;
}

class ITest_Logging : public testing::Test
{
protected:
    void SetUp() override
    {
        RemoveLogFiles();
    }

    void TearDown() override
    {
        RemoveLogFiles();
    }

    static void RemoveLogFiles()
    {
        for (const auto& logName : {simpleLogName, jsonLogName})
        {
            for (const auto& logFile : FindLogFiles(LogFilePrefix(logName)))
            {
                std::error_code ec;
                fs::remove(logFile, ec);
            }
        }
    }

    static auto ReadLogFile(const std::string& logName) -> std::string
    {
        const auto logFiles = FindLogFiles(LogFilePrefix(logName));
        EXPECT_EQ(logFiles.size(), 1u) << "Expected exactly one log file for '" << logName << "'";
        if (logFiles.size() != 1u)
        {
            return {};
        }

        std::ifstream stream{logFiles.front()};
        EXPECT_TRUE(stream.good()) << "Cannot open " << logFiles.front().string();

        std::stringstream contents;
        contents << stream.rdbuf();
        return contents.str();
    }
};

TEST_F(ITest_Logging, log_message_with_braces_is_not_parsed_as_format_striLogFilePrefixng)
{
    const auto bracedMessage = MakeBracedMessage();
    ASSERT_THAT(bracedMessage, testing::HasSubstr("{pattern=Wus, channel=A @ 12.796ms}"));

    // An unbalanced brace is the degenerate case: fmt cannot even recover by treating the field as named.
    const std::string unbalancedMessage{"A lone opening brace { and a lone closing brace }"};

    {
        auto registryConfig = SilKit::Config::ParticipantConfigurationFromString("");
        auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfig);
        const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

        auto participantConfig = SilKit::Config::ParticipantConfigurationFromString(MakeParticipantConfiguration());
        auto participant = SilKit::CreateParticipant(participantConfig, participantName, registryUri);

        auto* logger = participant->GetLogger();
        ASSERT_NE(logger, nullptr);

        EXPECT_NO_THROW(logger->Info(bracedMessage));
        EXPECT_NO_THROW(logger->Log(Level::Warn, bracedMessage));
        EXPECT_NO_THROW(logger->Error(unbalancedMessage));
    }
    // The participant is gone, so the file sinks are flushed and closed.

    const auto simpleLog = ReadLogFile(simpleLogName);
    EXPECT_THAT(simpleLog, testing::HasSubstr(bracedMessage));
    EXPECT_THAT(simpleLog, testing::HasSubstr(unbalancedMessage));

    const auto jsonLog = ReadLogFile(jsonLogName);
    EXPECT_THAT(jsonLog, testing::HasSubstr(bracedMessage));
    EXPECT_THAT(jsonLog, testing::HasSubstr(unbalancedMessage));
}

// Creates a participant with the given configuration, logs one message per level and destroys the participant again,
// which flushes and closes its file sinks.
void LogOneMessagePerLevel(const std::string& config, const std::string& name)
{
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    auto participant =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(config), name, registryUri);
    auto* logger = participant->GetLogger();
    logger->Trace("message-at-level-trace");
    logger->Debug("message-at-level-debug");
    logger->Info("message-at-level-info");
    logger->Warn("message-at-level-warn");
    logger->Error("message-at-level-error");
    logger->Critical("message-at-level-critical");
}

// Each sink only receives messages at or above its level.
TEST_F(ITest_Logging, sink_level_filters_messages_below_the_configured_level)
{
    IntegrationTestUtils::ScopedLogFiles warnLog{IntegrationTestUtils::MakeUniqueLogName("ITest_LogLevel_Warn")};
    IntegrationTestUtils::ScopedLogFiles traceLog{
        IntegrationTestUtils::MakeUniqueLogName("ITest_LogLevel_Trace")};

    LogOneMessagePerLevel(R"(
Logging:
  FlushLevel: Trace
  Sinks:
  - Type: File
    Level: Warn
    LogName: )" + warnLog.Prefix() + R"(
  - Type: File
    Level: Trace
    LogName: )" + traceLog.Prefix() + "\n",
                          participantName);

    const auto warn = warnLog.ReadAll();
    EXPECT_THAT(warn, testing::Not(testing::HasSubstr("message-at-level-trace")));
    EXPECT_THAT(warn, testing::Not(testing::HasSubstr("message-at-level-debug")));
    EXPECT_THAT(warn, testing::Not(testing::HasSubstr("message-at-level-info")));
    EXPECT_THAT(warn, testing::HasSubstr("message-at-level-warn"));
    EXPECT_THAT(warn, testing::HasSubstr("message-at-level-error"));
    EXPECT_THAT(warn, testing::HasSubstr("message-at-level-critical"));

    // The second sink of the same participant is not affected by the level of the first one.
    const auto trace = traceLog.ReadAll();
    for (const auto* level : {"trace", "debug", "info", "warn", "error", "critical"})
    {
        EXPECT_THAT(trace, testing::HasSubstr(std::string{"message-at-level-"} + level));
    }
}

TEST_F(ITest_Logging, sink_level_off_writes_nothing)
{
    IntegrationTestUtils::ScopedLogFiles offLog{IntegrationTestUtils::MakeUniqueLogName("ITest_LogLevel_Off")};

    LogOneMessagePerLevel(R"(
Logging:
  Sinks:
  - Type: File
    Level: Off
    LogName: )" + offLog.Prefix() + "\n",
                          participantName);

    EXPECT_THAT(offLog.ReadAll(), testing::Not(testing::HasSubstr("message-at-level-")));
}

// Additional file sink: the file is named
// "<LogName>_<ParticipantName>_<Timestamp>.jsonl" (File sinks default to the Json format) and each participant writes
// its own file.
TEST_F(ITest_Logging, file_sink_writes_one_file_per_participant_named_after_log_name_and_participant)
{
    IntegrationTestUtils::ScopedLogFiles fileLog{IntegrationTestUtils::MakeUniqueLogName("ITest_FileSink")};
    const auto config = R"(
Logging:
  Sinks:
  - Type: Stdout
    Level: Info
  - Type: File
    Level: Info
    LogName: )" + fileLog.Prefix() + "\n";

    LogOneMessagePerLevel(config, "FileSinkParticipantA");
    LogOneMessagePerLevel(config, "FileSinkParticipantB");

    const auto logFiles = IntegrationTestUtils::FindLogFiles(fileLog.Prefix());
    ASSERT_EQ(logFiles.size(), 2u);
    for (const auto& name : {"FileSinkParticipantA", "FileSinkParticipantB"})
    {
        const auto expectedPrefix = fileLog.Prefix() + "_" + name + "_";
        const auto logFile = std::find_if(logFiles.begin(), logFiles.end(), [&expectedPrefix](const fs::path& path) {
            return path.filename().string().rfind(expectedPrefix, 0) == 0;
        });
        ASSERT_NE(logFile, logFiles.end()) << "No log file starting with " << expectedPrefix;
        EXPECT_EQ(logFile->extension().string(), ".jsonl");
        EXPECT_THAT(IntegrationTestUtils::ReadTextFile(*logFile), testing::HasSubstr("message-at-level-info"));
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// Remote logging
// ---------------------------------------------------------------------------------------------------------------------

const std::string sender1InfoMessage = "remote-log-from-sender-1";
const std::string sender2InfoMessage = "remote-log-from-sender-2";
const std::string sender1WarnMessage = "remote-warning-from-sender-1";
const std::string receiverOwnMessage = "local-log-from-receiver";

// Runs Sender1, Sender2 and Receiver for a few simulation steps. In their first step, the senders log an info message
// and Sender1 additionally a warning; the receiver logs a message of its own. Returns the receiver's file sink contents.
auto RunRemoteLoggingSimulation(const std::string& senderConfig, const std::string& receiverLoggingSection)
    -> std::string
{
    const auto receiverLogName = IntegrationTestUtils::MakeUniqueLogName("itest_remote_logging");
    const auto filePrefix = LogFilePrefix(receiverLogName, "Receiver");
    IntegrationTestUtils::ScopedLogFiles cleanup{filePrefix};

    const auto receiverConfig = R"(
Logging:
)" + receiverLoggingSection + R"(
  FlushLevel: Trace
  Sinks:
    - Type: File
      Level: Trace
      LogName: )" + receiverLogName + "\n";

    SilKit::Tests::SimTestHarnessArgs testHarnessArgs;
    testHarnessArgs.syncParticipantNames = {"Sender1", "Sender2", "Receiver"};
    testHarnessArgs.deferParticipantCreation = true;

    SilKit::Tests::SimTestHarness testHarness{testHarnessArgs};

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

    const auto logFiles = IntegrationTestUtils::FindLogFiles(filePrefix);
    EXPECT_EQ(logFiles.size(), 1u) << "Expected exactly one receiver log file with prefix " << filePrefix;
    return logFiles.empty() ? std::string{} : IntegrationTestUtils::ReadTextFile(logFiles.front());
}

const std::string remoteTraceSenderConfig = R"(
Logging:
  Sinks:
    - Type: Remote
      Level: Trace
)";

TEST_F(ITest_Logging, remote_logging_two_senders_one_receiver)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "  LogFromRemotes: true");

    EXPECT_THAT(logContent, testing::HasSubstr(sender1InfoMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(sender1WarnMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(sender2InfoMessage));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

// A participant that does not set LogFromRemotes must not receive the log messages of other participants.
TEST_F(ITest_Logging, log_from_remotes_false_ignores_remote_messages)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "  LogFromRemotes: false");

    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1InfoMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1WarnMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender2InfoMessage)));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

TEST_F(ITest_Logging, log_from_remotes_defaults_to_false)
{
    const auto logContent = RunRemoteLoggingSimulation(remoteTraceSenderConfig, "");

    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender1InfoMessage)));
    EXPECT_THAT(logContent, testing::Not(testing::HasSubstr(sender2InfoMessage)));
    EXPECT_THAT(logContent, testing::HasSubstr(receiverOwnMessage));
}

// The level of the sender's Remote sink decides which messages are sent at all.
TEST_F(ITest_Logging, remote_sink_level_filters_on_the_sender_side)
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
