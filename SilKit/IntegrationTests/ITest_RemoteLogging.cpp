// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <system_error>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "silkit/services/all.hpp"

#include "SimTestHarness.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Tests;

auto ReadTextFile(const std::filesystem::path& filePath) -> std::string
{
    std::ifstream in{filePath};
    return std::string{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

auto FindLogFiles(const std::string& logNamePrefix) -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> candidates;

    for (const auto& entry : std::filesystem::directory_iterator{std::filesystem::current_path()})
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const auto filename = entry.path().filename().string();
        if (filename.rfind(logNamePrefix, 0) == 0 && entry.path().extension() == ".jsonl")
        {
            candidates.emplace_back(entry.path());
        }
    }

    return candidates;
}

auto FindLogFile(const std::string& logNamePrefix) -> std::filesystem::path
{
    const auto candidates = FindLogFiles(logNamePrefix);

    if (candidates.size() != 1u)
    {
        return {};
    }

    return candidates.front();
}

struct ScopedLogFileCleanup
{
    explicit ScopedLogFileCleanup(std::string prefix)
        : _prefix{std::move(prefix)}
    {
    }

    ~ScopedLogFileCleanup()
    {
        for (const auto& entry : std::filesystem::directory_iterator{std::filesystem::current_path()})
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const auto filename = entry.path().filename().string();
            if (filename.rfind(_prefix, 0) == 0 && entry.path().extension() == ".jsonl")
            {
                std::error_code ec;
                std::filesystem::remove(entry.path(), ec);
            }
        }
    }

    std::string _prefix;
};

TEST(ITest_RemoteLogging, test_remote_logging_two_senders_one_receiver)
{
    const auto uniqueSuffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto receiverLogName = "itest_remote_logging_" + uniqueSuffix;
    const auto filePrefix = receiverLogName + "_Receiver_";
    ScopedLogFileCleanup cleanup{filePrefix};

    const auto senderConfig = R"(
Logging:
  Sinks:
    - Type: Remote
      Level: Trace
)";

    const auto receiverConfig = R"(
Logging:
  LogFromRemotes: true
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

    const std::string sender1Message = "remote-log-from-sender-1";
    const std::string sender2Message = "remote-log-from-sender-2";

    bool sender1Logged{false};
    bool sender2Logged{false};

    sender1TimeSync->SetSimulationStepHandler([&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (!sender1Logged)
        {
            sender1Logged = true;
            sender1Logger->Info(sender1Message);
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
            sender2Logger->Info(sender2Message);
        }
    },
        1ms);

    receiverTimeSync->SetSimulationStepHandler([](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds) {}, 1ms);

    ASSERT_TRUE(testHarness.Run(5s));

    const auto deadline = std::chrono::steady_clock::now() + 10s;
    std::filesystem::path logFile;
    std::string logContent;

    while (std::chrono::steady_clock::now() < deadline)
    {
        logFile = FindLogFile(filePrefix);
        if (!logFile.empty())
        {
            logContent = ReadTextFile(logFile);
            if (logContent.find(sender1Message) != std::string::npos
                && logContent.find(sender2Message) != std::string::npos)
            {
                break;
            }
        }

        std::this_thread::sleep_for(10ms);
    }

    ASSERT_FALSE(logFile.empty()) << "Could not find exactly one receiver log file with prefix " << filePrefix << "\n";
    EXPECT_NE(logContent.find(sender1Message), std::string::npos);
    EXPECT_NE(logContent.find(sender2Message), std::string::npos);
}

} // namespace
