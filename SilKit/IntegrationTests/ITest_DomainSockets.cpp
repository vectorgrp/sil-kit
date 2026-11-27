// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Participants on the same host connect via local domain sockets by default, and via TCP if domain sockets are
// disabled.

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "IntegrationTestUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using IntegrationTestUtils::MakeUniqueLogName;
using IntegrationTestUtils::ScopedLogFiles;
using namespace std::chrono_literals;

auto MakeConfig(const std::string& logName, const std::string& middleware) -> std::string
{
    return R"(
Logging:
  FlushLevel: Trace
  Sinks:
  - Type: File
    Format: Simple
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

auto JoinWithAcceptorUris(bool session, bool enableDomainSockets, bool fixedTcpPort) -> std::string
{
    ScopedLogFiles logs{MakeUniqueLogName("FixedAcceptors")};
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");
    const auto localUri = "local://" + logs.Prefix() + ".sock";
    auto middleware = std::string{"\nMiddleware:\n  EnableDomainSockets: "} + (enableDomainSockets ? "true" : "false")
                      + "\n  AcceptorUris:\n  - tcp://127.0.0.1:0\n  - " + localUri + "\n";
    if (fixedTcpPort)
    {
        // Use the registry's port to exercise a known bind collision without reserving a hard-coded port.
        middleware += "  - tcp://127.0.0.1" + registryUri.substr(registryUri.rfind(':')) + "\n";
    }
    const auto config = SilKit::Config::ParticipantConfigurationFromString(MakeConfig(logs.Prefix(), middleware));
    {
        auto participant = SilKit::CreateParticipant(config, "TestParticipant",
                                                     registryUri + (session ? "/session/fixed" : "/fixed"));
    }
    return logs.ReadAll();
}

TEST(ITest_DomainSockets, registry_session_warns_once_about_fixed_acceptors_before_binding)
{
    const auto log = JoinWithAcceptorUris(true, true, true);
    const auto warning = log.find("with fixed acceptor endpoints: ");
    ASSERT_NE(warning, std::string::npos) << log;
    const auto warningText = log.substr(warning, log.find('\n', warning) - warning);
    EXPECT_THAT(warningText, testing::HasSubstr("local://FixedAcceptors"));
    EXPECT_THAT(warningText, testing::HasSubstr("tcp://127.0.0.1:"));
    EXPECT_THAT(warningText, testing::Not(testing::HasSubstr("tcp://127.0.0.1:0")));
    EXPECT_EQ(log.find("with fixed acceptor endpoints: ", warning + 1), std::string::npos);
    const auto bindError = log.find("Unable to accept TCP connections on");
    ASSERT_NE(bindError, std::string::npos) << log;
    EXPECT_LT(warning, bindError);
}

TEST(ITest_DomainSockets, registry_session_does_not_warn_for_dynamic_tcp_or_disabled_domain_sockets)
{
    EXPECT_THAT(JoinWithAcceptorUris(true, false, false),
                testing::Not(testing::HasSubstr("with fixed acceptor endpoints: ")));
}

TEST(ITest_DomainSockets, non_session_does_not_warn_about_fixed_acceptors)
{
    EXPECT_THAT(JoinWithAcceptorUris(false, true, true),
                testing::Not(testing::HasSubstr("with fixed acceptor endpoints: ")));
}

struct ReceivedData
{
    void Add(const SilKit::Services::PubSub::DataMessageEvent& event)
    {
        std::lock_guard<std::mutex> lock{mutex};
        messages.emplace_back(event.data.begin(), event.data.end());
        changed.notify_all();
    }

    auto WaitFor(uint8_t value) -> bool
    {
        std::unique_lock<std::mutex> lock{mutex};
        return changed.wait_for(lock, 5s, [&] {
            return std::find(messages.begin(), messages.end(), std::vector<uint8_t>{value}) != messages.end();
        });
    }

    std::mutex mutex;
    std::condition_variable changed;
    std::vector<std::vector<uint8_t>> messages;
};

struct SocketGeneration
{
    SocketGeneration(const std::shared_ptr<SilKit::Config::IParticipantConfiguration>& config,
                     const std::string& registryUri, ReceivedData& received)
    {
        publisherParticipant = SilKit::CreateParticipant(config, "Publisher", registryUri);
        publisher = publisherParticipant->CreateDataPublisher("Publisher", spec, 1);
        publisherLifecycle = publisherParticipant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});

        subscriberParticipant = SilKit::CreateParticipant(config, "Subscriber", registryUri);
        subscriberParticipant->CreateDataSubscriber("Subscriber", spec,
                                                    [&received](auto*, const auto& event) { received.Add(event); });
        subscriberLifecycle = subscriberParticipant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});

        controllerParticipant = SilKit::CreateParticipant(config, "Controller", registryUri);
        controllerParticipant->CreateSystemMonitor()->AddParticipantStatusHandler([this](const auto& status) {
            if (status.state == SilKit::Services::Orchestration::ParticipantState::Running)
            {
                if (status.participantName == "Publisher")
                {
                    publisherRunning.set_value();
                }
                else if (status.participantName == "Subscriber")
                {
                    subscriberRunning.set_value();
                }
            }
        });
        SilKit::Experimental::Participant::CreateSystemController(controllerParticipant.get())
            ->SetWorkflowConfiguration({{"Publisher", "Subscriber"}});
    }

    void Start()
    {
        auto publisherReady = publisherRunning.get_future();
        auto subscriberReady = subscriberRunning.get_future();
        publisherDone = publisherLifecycle->StartLifecycle();
        subscriberDone = subscriberLifecycle->StartLifecycle();
        ASSERT_EQ(publisherReady.wait_for(5s), std::future_status::ready);
        ASSERT_EQ(subscriberReady.wait_for(5s), std::future_status::ready);
    }

    void Stop()
    {
        publisherLifecycle->Stop("Socket isolation test");
        ASSERT_EQ(publisherDone.wait_for(5s), std::future_status::ready);
        ASSERT_EQ(subscriberDone.wait_for(5s), std::future_status::ready);
        EXPECT_EQ(publisherDone.get(), SilKit::Services::Orchestration::ParticipantState::Shutdown);
        EXPECT_EQ(subscriberDone.get(), SilKit::Services::Orchestration::ParticipantState::Shutdown);
    }

    void Publish(uint8_t value)
    {
        const std::vector<uint8_t> data{value};
        publisher->Publish(SilKit::Util::ToSpan(data));
    }

    SilKit::Services::PubSub::PubSubSpec spec{"SocketIsolation", "application/octet-stream"};
    std::promise<void> publisherRunning;
    std::promise<void> subscriberRunning;
    std::future<SilKit::Services::Orchestration::ParticipantState> publisherDone;
    std::future<SilKit::Services::Orchestration::ParticipantState> subscriberDone;
    SilKit::Services::PubSub::IDataPublisher* publisher{nullptr};
    SilKit::Services::Orchestration::ILifecycleService* publisherLifecycle{nullptr};
    SilKit::Services::Orchestration::ILifecycleService* subscriberLifecycle{nullptr};
    std::unique_ptr<SilKit::IParticipant> publisherParticipant;
    std::unique_ptr<SilKit::IParticipant> subscriberParticipant;
    std::unique_ptr<SilKit::IParticipant> controllerParticipant;
};

auto ParticipantSocketPaths(const std::string& log) -> std::set<std::string>
{
    std::set<std::string> paths;
    std::istringstream lines{log};
    std::string line;
    const std::regex localPath{R"(UriPath: (.*\.silkit))"};
    while (std::getline(lines, line))
    {
        if (line.find("Found local domain acceptor endpoint URI") == std::string::npos)
        {
            continue;
        }
        std::smatch match;
        const auto matched = std::regex_search(line, match, localPath);
        EXPECT_TRUE(matched) << line;
        if (matched)
        {
            paths.insert(match[1].str());
        }
    }
    return paths;
}

void CheckGenerationIsolation(bool removeOldest)
{
    ScopedLogFiles firstLogs{MakeUniqueLogName("ITest_DomainSockets_First")};
    ScopedLogFiles secondLogs{MakeUniqueLogName("ITest_DomainSockets_Second")};
    ScopedLogFiles replacementLogs{MakeUniqueLogName("ITest_DomainSockets_Replacement")};
    ScopedLogFiles lateLogs{MakeUniqueLogName("ITest_DomainSockets_Late")};
    const auto configFor = [](const ScopedLogFiles& logs) {
        return SilKit::Config::ParticipantConfigurationFromString(MakeConfig(logs.Prefix(), ""));
    };
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0") + "/session/socket-isolation";
    ReceivedData firstReceived;
    ReceivedData secondReceived;
    ReceivedData replacementReceived;
    ReceivedData lateReceived;

    auto first = std::make_unique<SocketGeneration>(configFor(firstLogs), registryUri, firstReceived);
    ASSERT_NO_FATAL_FAILURE(first->Start());
    first->Publish(1);
    ASSERT_TRUE(firstReceived.WaitFor(1));

    auto second = std::make_unique<SocketGeneration>(configFor(secondLogs), registryUri, secondReceived);
    ASSERT_NO_FATAL_FAILURE(second->Start());
    second->Publish(2);
    ASSERT_TRUE(secondReceived.WaitFor(2));
    first->Publish(3);
    second->Publish(4);
    ASSERT_TRUE(firstReceived.WaitFor(3));
    ASSERT_TRUE(secondReceived.WaitFor(4));

    const auto firstPaths = ParticipantSocketPaths(firstLogs.ReadAll());
    const auto secondPaths = ParticipantSocketPaths(secondLogs.ReadAll());
    ASSERT_EQ(firstPaths.size(), 3u);
    ASSERT_EQ(secondPaths.size(), 3u);
    for (const auto& path : firstPaths)
    {
        EXPECT_EQ(secondPaths.count(path), 0u);
    }

    auto& removed = removeOldest ? first : second;
    auto& survivor = removeOldest ? second : first;
    auto& survivorReceived = removeOldest ? secondReceived : firstReceived;
    const auto& survivorPaths = removeOldest ? secondPaths : firstPaths;
    ASSERT_NO_FATAL_FAILURE(removed->Stop());
    removed.reset();
    EXPECT_EQ(survivor->publisherLifecycle->State(), SilKit::Services::Orchestration::ParticipantState::Running);
    EXPECT_EQ(survivor->subscriberLifecycle->State(), SilKit::Services::Orchestration::ParticipantState::Running);
    for (const auto& path : survivorPaths)
    {
        EXPECT_TRUE(std::filesystem::exists(path)) << path;
    }

    survivor->Publish(5);
    ASSERT_TRUE(survivorReceived.WaitFor(5));
    auto late = SilKit::CreateParticipant(configFor(lateLogs), "LateSubscriber", registryUri);
    late->CreateDataSubscriber("LateSubscriber", survivor->spec,
                               [&lateReceived](auto*, const auto& event) { lateReceived.Add(event); });
    ASSERT_TRUE(lateReceived.WaitFor(5));
    survivor->Publish(6);
    ASSERT_TRUE(survivorReceived.WaitFor(6));
    ASSERT_TRUE(lateReceived.WaitFor(6));

    auto replacement = std::make_unique<SocketGeneration>(configFor(replacementLogs), registryUri, replacementReceived);
    ASSERT_NO_FATAL_FAILURE(replacement->Start());
    replacement->Publish(7);
    ASSERT_TRUE(replacementReceived.WaitFor(7));
    survivor->Publish(8);
    ASSERT_TRUE(survivorReceived.WaitFor(8));
    ASSERT_TRUE(lateReceived.WaitFor(8));

    ASSERT_NO_FATAL_FAILURE(replacement->Stop());
    replacement.reset();
    ASSERT_NO_FATAL_FAILURE(survivor->Stop());
    survivor.reset();
    late.reset();

    using testing::ElementsAre;
    if (removeOldest)
    {
        EXPECT_THAT(firstReceived.messages, ElementsAre(ElementsAre(1), ElementsAre(3)));
        EXPECT_THAT(secondReceived.messages,
                    ElementsAre(ElementsAre(2), ElementsAre(4), ElementsAre(5), ElementsAre(6), ElementsAre(8)));
    }
    else
    {
        EXPECT_THAT(firstReceived.messages,
                    ElementsAre(ElementsAre(1), ElementsAre(3), ElementsAre(5), ElementsAre(6), ElementsAre(8)));
        EXPECT_THAT(secondReceived.messages, ElementsAre(ElementsAre(2), ElementsAre(4)));
    }
    EXPECT_THAT(replacementReceived.messages, ElementsAre(ElementsAre(7)));
    EXPECT_THAT(lateReceived.messages, ElementsAre(ElementsAre(5), ElementsAre(6), ElementsAre(8)));

    for (const auto* logs : {&firstLogs, &secondLogs, &replacementLogs, &lateLogs})
    {
        const auto log = logs->ReadAll();
        EXPECT_THAT(log, testing::HasSubstr("Connected to registry at '"));
        EXPECT_THAT(log, testing::Not(testing::HasSubstr("New connection from [local=tcp://")));
    }
    EXPECT_THAT(firstLogs.ReadAll(), testing::HasSubstr("New connection from [local=local://"));
    EXPECT_THAT(secondLogs.ReadAll(), testing::HasSubstr("New connection from [local=local://"));
}

TEST(ITest_DomainSockets, generations_remain_isolated_when_the_older_generation_is_removed)
{
    CheckGenerationIsolation(true);
}

TEST(ITest_DomainSockets, generations_remain_isolated_when_the_newer_generation_is_removed)
{
    CheckGenerationIsolation(false);
}

} // namespace
