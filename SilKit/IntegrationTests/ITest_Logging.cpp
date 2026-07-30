// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

auto MakeParticipantConfiguration() -> std::string
{
    std::stringstream config;
    config << R"({"Logging":{"FlushLevel":"Trace","Sinks":[)"
           << R"({"Type":"File","Format":"Simple","Level":"Trace","LogName":")" << simpleLogName << R"("},)"
           << R"({"Type":"File","Format":"Json","Level":"Trace","LogName":")" << jsonLogName << R"("}]}})";
    return config.str();
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
        const auto prefix = logName + "_";

        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator{fs::current_path()})
        {
            if (entry.is_regular_file() && entry.path().filename().string().compare(0, prefix.size(), prefix) == 0)
            {
                logFiles.push_back(entry.path());
            }
        }
        return logFiles;
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

} // namespace
