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
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "ITestLogFiles.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace {

namespace fs = std::filesystem;

using namespace std::chrono_literals;
using namespace SilKit::Services::Logging;

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

    // The sinks append a participant name and a timestamp to the configured log name, so the files can only
    // be identified by their prefix.
    static auto FindLogFiles(const std::string& logName) -> std::vector<fs::path>
    {
        return SilKit::IntegrationTests::FindLogFiles(logName + "_");
    }

    static void RemoveLogFiles()
    {
        for (const auto& logName : {simpleLogName, jsonLogName})
        {
            for (const auto& logFile : FindLogFiles(logName))
            {
                std::error_code ec;
                fs::remove(logFile, ec);
            }
        }
    }

    static auto ReadLogFile(const std::string& logName) -> std::string
    {
        const auto logFiles = FindLogFiles(logName);
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

TEST_F(ITest_Logging, log_message_with_braces_is_not_parsed_as_format_string)
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

// QA test case 2.6 "Logger Configuration - Log Level": each sink only receives messages at or above its level.
TEST_F(ITest_Logging, sink_level_filters_messages_below_the_configured_level)
{
    SilKit::IntegrationTests::ScopedLogFiles warnLog{SilKit::IntegrationTests::MakeUniqueLogName("ITest_LogLevel_Warn")};
    SilKit::IntegrationTests::ScopedLogFiles traceLog{
        SilKit::IntegrationTests::MakeUniqueLogName("ITest_LogLevel_Trace")};

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
    SilKit::IntegrationTests::ScopedLogFiles offLog{SilKit::IntegrationTests::MakeUniqueLogName("ITest_LogLevel_Off")};

    LogOneMessagePerLevel(R"(
Logging:
  Sinks:
  - Type: File
    Level: Off
    LogName: )" + offLog.Prefix() + "\n",
                          participantName);

    EXPECT_THAT(offLog.ReadAll(), testing::Not(testing::HasSubstr("message-at-level-")));
}

// QA test case 2.7 "Logger Configuration - Additional File Sink": the file is named
// "<LogName>_<ParticipantName>_<Timestamp>.jsonl" (File sinks default to the Json format) and each participant writes
// its own file.
TEST_F(ITest_Logging, file_sink_writes_one_file_per_participant_named_after_log_name_and_participant)
{
    SilKit::IntegrationTests::ScopedLogFiles fileLog{SilKit::IntegrationTests::MakeUniqueLogName("ITest_FileSink")};
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

    const auto logFiles = SilKit::IntegrationTests::FindLogFiles(fileLog.Prefix());
    ASSERT_EQ(logFiles.size(), 2u);
    for (const auto& name : {"FileSinkParticipantA", "FileSinkParticipantB"})
    {
        const auto expectedPrefix = fileLog.Prefix() + "_" + name + "_";
        const auto logFile = std::find_if(logFiles.begin(), logFiles.end(), [&expectedPrefix](const fs::path& path) {
            return path.filename().string().rfind(expectedPrefix, 0) == 0;
        });
        ASSERT_NE(logFile, logFiles.end()) << "No log file starting with " << expectedPrefix;
        EXPECT_EQ(logFile->extension().string(), ".jsonl");
        EXPECT_THAT(SilKit::IntegrationTests::ReadTextFile(*logFile), testing::HasSubstr("message-at-level-info"));
    }
}

} // namespace
