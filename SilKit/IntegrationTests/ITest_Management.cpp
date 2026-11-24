// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/experimental/management/ManagementClient.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Experimental::Management;
using SilKit::Services::Rpc::RpcCallStatus;
using testing::Each;
using testing::ElementsAre;
using testing::HasSubstr;
using testing::IsEmpty;

auto Names(const Simulations& simulations) -> std::vector<std::string>
{
    std::vector<std::string> names;
    for (const auto& simulation : simulations.simulations)
    {
        names.push_back(simulation.name);
    }
    return names;
}

auto Find(const Simulations& simulations, const std::string& name) -> const SimulationInfo*
{
    for (const auto& simulation : simulations.simulations)
    {
        if (simulation.name == name)
        {
            return &simulation;
        }
    }
    return nullptr;
}

struct SimulationsObserver
{
    std::mutex mutex;
    std::condition_variable condition;
    Simulations simulations;
    std::vector<SimulationsChange> changes;

    void Attach(ManagementClient& client)
    {
        client.SetSimulationsHandler([this](const Simulations& newSimulations, const SimulationsChange& change) {
            std::lock_guard<std::mutex> lock{mutex};
            simulations = newSimulations;
            changes.push_back(change);
            condition.notify_all();
        });
    }

    bool WaitFor(std::function<bool(const Simulations&)> predicate)
    {
        std::unique_lock<std::mutex> lock{mutex};
        return condition.wait_for(lock, 10s, [this, &predicate] { return predicate(simulations); });
    }
};

auto RegistryInfoFuture(ManagementClient& client) -> std::future<RegistryInfo>
{
    auto promise = std::make_shared<std::promise<RegistryInfo>>();
    auto future = promise->get_future();
    auto called = std::make_shared<std::atomic<bool>>(false);
    client.SetRegistryInfoHandler([promise, called](const RegistryInfo& registryInfo) {
        if (!called->exchange(true))
        {
            promise->set_value(registryInfo);
        }
    });
    return future;
}

auto GetParticipantConfiguration(ManagementClient& client, const std::string& simulationName,
                                 const std::string& participantName) -> ParticipantConfigurationReply
{
    // the RPC server of the management participant may not be discovered yet right after joining
    for (int attempt = 0;; ++attempt)
    {
        std::promise<std::pair<SilKit::Services::Rpc::RpcCallStatus, ParticipantConfigurationReply>> promise;
        client.GetParticipantConfiguration(
            simulationName, participantName, 2s,
            [&promise](SilKit::Services::Rpc::RpcCallStatus callStatus, const ParticipantConfigurationReply& reply) {
            promise.set_value({callStatus, reply});
        });
        const auto result = promise.get_future().get();
        if (result.first == SilKit::Services::Rpc::RpcCallStatus::Success || attempt == 10)
        {
            EXPECT_EQ(result.first, SilKit::Services::Rpc::RpcCallStatus::Success);
            return result.second;
        }
        std::this_thread::sleep_for(100ms);
    }
}

//! Results of participant configuration requests sent to a server that never answers.
struct UnansweredRequests
{
    std::mutex mutex;
    std::condition_variable condition;
    bool serverCalled{false};
    std::vector<RpcCallStatus> results;

    void OnResult(RpcCallStatus callStatus)
    {
        std::lock_guard<std::mutex> lock{mutex};
        results.push_back(callStatus);
        condition.notify_all();
    }

    //! Sends requests until one reaches the server; the ones before fail as long as the server is not discovered.
    bool SendUntilServerCalled(const std::function<void()>& sendRequest)
    {
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            std::unique_lock<std::mutex> lock{mutex};
            const auto numResults = results.size();
            lock.unlock();

            sendRequest();

            lock.lock();
            condition.wait_for(lock, 5s, [this, numResults] { return serverCalled || results.size() > numResults; });
            if (serverCalled)
            {
                return true;
            }
            lock.unlock();
            std::this_thread::sleep_for(10ms);
        }
        return false;
    }
};

//! Joins the management simulation and accepts participant configuration requests without ever answering them.
auto CreateSilentServer(const std::string& registryUri, UnansweredRequests& requests)
    -> std::unique_ptr<SilKit::IParticipant>
{
    auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 "silent-server", MakeManagementUri(registryUri));
    participant->CreateRpcServer(
        "SilentServer", SilKit::Services::Rpc::RpcSpec{Registry::GetParticipantConfigurationFunction(), MediaType()},
        [&requests](SilKit::Services::Rpc::IRpcServer*, const SilKit::Services::Rpc::RpcCallEvent&) {
        std::lock_guard<std::mutex> lock{requests.mutex};
        requests.serverCalled = true;
        requests.condition.notify_all();
    });
    return participant;
}

TEST(ITest_Management, management_participant_reports_simulations_required_participants_and_configurations)
{
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(R"(
Experimental:
  Management:
    Enable: true
)"));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    const auto defaultConfiguration = SilKit::Config::ParticipantConfigurationFromString("");
    const auto sharingConfiguration = SilKit::Config::ParticipantConfigurationFromString(R"(
Experimental:
  Management:
    ShareConfiguration: true
)");

    // declared before the client, which calls into it until the client is destroyed
    SimulationsObserver observer;
    auto client = CreateManagementClient(defaultConfiguration, registryUri);

    auto registryInfoFuture = RegistryInfoFuture(*client);
    ASSERT_EQ(registryInfoFuture.wait_for(10s), std::future_status::ready);
    const auto registryInfo = registryInfoFuture.get();
    EXPECT_EQ(registryInfo.apiVersionMajor, ApiVersionMajor());
    EXPECT_THAT(registryInfo.features,
                ElementsAre(Registry::FeatureSimulations(), Registry::FeatureParticipantConfiguration()));

    observer.Attach(*client);

    auto a1 = SilKit::CreateParticipant(sharingConfiguration, "A1", registryUri + "/a");
    auto a2 = SilKit::CreateParticipant(defaultConfiguration, "A2", registryUri + "/a");
    auto b1 = SilKit::CreateParticipant(defaultConfiguration, "B1", registryUri + "/b");

    ASSERT_TRUE(observer.WaitFor([](const Simulations& simulations) {
        const auto* a = Find(simulations, "a");
        return Names(simulations) == std::vector<std::string>{"a", "b"} && a->participantNames.size() == 2;
    }));

    auto* systemController = SilKit::Experimental::Participant::CreateSystemController(a1.get());
    systemController->SetWorkflowConfiguration(SilKit::Services::Orchestration::WorkflowConfiguration{{"A1", "A2"}});

    ASSERT_TRUE(observer.WaitFor([](const Simulations& simulations) {
        const auto* a = Find(simulations, "a");
        return a != nullptr && a->requiredParticipantNames == std::vector<std::string>{"A1", "A2"};
    }));
    {
        std::lock_guard<std::mutex> lock{observer.mutex};
        EXPECT_THAT(Find(observer.simulations, "b")->requiredParticipantNames, IsEmpty());
    }

    const auto shared = GetParticipantConfiguration(*client, "a", "A1");
    EXPECT_EQ(shared.status, ParticipantConfigurationStatus::Available);
    EXPECT_THAT(shared.configurationJson, HasSubstr("ShareConfiguration"));

    EXPECT_EQ(GetParticipantConfiguration(*client, "a", "A2").status, ParticipantConfigurationStatus::NotShared);
    EXPECT_EQ(GetParticipantConfiguration(*client, "a", "Unknown").status,
              ParticipantConfigurationStatus::UnknownParticipant);

    b1.reset();

    ASSERT_TRUE(observer.WaitFor(
        [](const Simulations& simulations) { return Names(simulations) == std::vector<std::string>{"a"}; }));

    std::lock_guard<std::mutex> lock{observer.mutex};
    EXPECT_THAT(observer.changes.back().removed, ElementsAre("b"));
}

TEST(ITest_Management, management_participant_does_not_count_as_connected_participant)
{
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(R"(
Experimental:
  Management:
    Enable: true
)"));

    std::atomic<bool> allDisconnectedCalled{false};
    std::promise<void> allDisconnectedPromise;
    registry->SetAllDisconnectedHandler([&allDisconnectedCalled, &allDisconnectedPromise] {
        if (!allDisconnectedCalled.exchange(true))
        {
            allDisconnectedPromise.set_value();
        }
    });

    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    auto participant =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "P", registryUri + "/a");
    participant.reset();

    EXPECT_EQ(allDisconnectedPromise.get_future().wait_for(10s), std::future_status::ready);
}

TEST(ITest_Management, registry_without_management_publishes_no_registry_info)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    auto client = CreateManagementClient(SilKit::Config::ParticipantConfigurationFromString(""), registryUri);

    EXPECT_EQ(RegistryInfoFuture(*client).wait_for(1s), std::future_status::timeout);
}

TEST(ITest_Management, destroying_the_client_completes_pending_requests)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    // declared before the server and the client, which call into it until they are destroyed
    UnansweredRequests requests;
    auto server = CreateSilentServer(registryUri, requests);
    auto client = CreateManagementClient(SilKit::Config::ParticipantConfigurationFromString(""), registryUri);

    ASSERT_TRUE(requests.SendUntilServerCalled([&client, &requests] {
        client->GetParticipantConfiguration(
            "a", "A1", 60s, [&requests](RpcCallStatus callStatus, const ParticipantConfigurationReply&) {
            requests.OnResult(callStatus);
        });
    }));

    size_t numResults{0};
    {
        std::lock_guard<std::mutex> lock{requests.mutex};
        EXPECT_THAT(requests.results, Each(RpcCallStatus::ServerNotReachable));
        numResults = requests.results.size();
    }

    client.reset();

    std::lock_guard<std::mutex> lock{requests.mutex};
    ASSERT_EQ(requests.results.size(), numResults + 1);
    EXPECT_EQ(requests.results.back(), RpcCallStatus::UndefinedError);
}

// ================================================================================
//  C API
// ================================================================================

auto ToVector(const SilKit_StringList* list) -> std::vector<std::string>
{
    return std::vector<std::string>(list->strings, list->strings + list->numStrings);
}

//! Destroys a C client also when an assertion ends the test early.
struct ClientDestroyer
{
    void operator()(SilKit_Experimental_ManagementClient* client) const
    {
        SilKit_Experimental_ManagementClient_Destroy(client);
    }
};

struct CClientState
{
    std::mutex mutex;
    std::condition_variable condition;

    bool hasRegistryInfo{false};
    uint32_t apiVersionMajor{0};
    std::vector<std::string> features;

    std::vector<std::string> simulationNames;
    std::vector<std::string> participantsOfA;
    std::vector<std::string> createdSimulationNames;

    bool hasReply{false};
    SilKit_RpcCallStatus callStatus{SilKit_RpcCallStatus_UndefinedError};
    SilKit_Experimental_ManagementParticipantConfigurationStatus status{
        SilKit_Experimental_ManagementParticipantConfigurationStatus_UnknownParticipant};
    std::string configurationJson;

    bool WaitFor(std::function<bool()> predicate)
    {
        std::unique_lock<std::mutex> lock{mutex};
        return condition.wait_for(lock, 10s, predicate);
    }
};

void SilKitCALL OnCRegistryInfo(void* context, SilKit_Experimental_ManagementClient*,
                                const SilKit_Experimental_ManagementRegistryInfo* registryInfo)
{
    auto* state = static_cast<CClientState*>(context);
    std::lock_guard<std::mutex> lock{state->mutex};
    state->hasRegistryInfo = true;
    state->apiVersionMajor = registryInfo->apiVersionMajor;
    state->features = ToVector(registryInfo->features);
    state->condition.notify_all();
}

void SilKitCALL OnCSimulations(void* context, SilKit_Experimental_ManagementClient*,
                               const SilKit_Experimental_ManagementSimulationsEvent* simulationsEvent)
{
    auto* state = static_cast<CClientState*>(context);
    std::lock_guard<std::mutex> lock{state->mutex};
    state->simulationNames.clear();
    for (size_t index = 0; index < simulationsEvent->numSimulations; ++index)
    {
        const auto& simulation = simulationsEvent->simulations[index];
        state->simulationNames.emplace_back(simulation.name);
        if (std::string{simulation.name} == "a")
        {
            state->participantsOfA = ToVector(simulation.participantNames);
        }
    }
    for (const auto& name : ToVector(simulationsEvent->createdSimulationNames))
    {
        state->createdSimulationNames.push_back(name);
    }
    state->condition.notify_all();
}

void SilKitCALL OnCParticipantConfiguration(void* context, SilKit_Experimental_ManagementClient*,
                                            SilKit_RpcCallStatus callStatus,
                                            const SilKit_Experimental_ManagementParticipantConfigurationReply* reply)
{
    auto* state = static_cast<CClientState*>(context);
    std::lock_guard<std::mutex> lock{state->mutex};
    state->hasReply = true;
    state->callStatus = callStatus;
    if (reply != nullptr)
    {
        state->status = reply->status;
        state->configurationJson = reply->configurationJson;
    }
    state->condition.notify_all();
}

TEST(ITest_Management, c_api_client)
{
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(R"(
Experimental:
  Management:
    Enable: true
)"));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    // declared before the client, which calls into it until the client is destroyed
    CClientState state;

    SilKit_ParticipantConfiguration* configuration{nullptr};
    ASSERT_EQ(SilKit_ParticipantConfiguration_FromString(&configuration, ""), SilKit_ReturnCode_SUCCESS);
    SilKit_Experimental_ManagementClient* client{nullptr};
    ASSERT_EQ(SilKit_Experimental_ManagementClient_Create(&client, configuration, registryUri.c_str()),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_ParticipantConfiguration_Destroy(configuration), SilKit_ReturnCode_SUCCESS);
    std::unique_ptr<SilKit_Experimental_ManagementClient, ClientDestroyer> clientGuard{client};

    ASSERT_EQ(SilKit_Experimental_ManagementClient_SetRegistryInfoHandler(client, &state, &OnCRegistryInfo),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_Experimental_ManagementClient_SetSimulationsHandler(client, &state, &OnCSimulations),
              SilKit_ReturnCode_SUCCESS);

    ASSERT_TRUE(state.WaitFor([&state] { return state.hasRegistryInfo; }));
    EXPECT_EQ(state.apiVersionMajor, ApiVersionMajor());
    EXPECT_THAT(state.features,
                ElementsAre(Registry::FeatureSimulations(), Registry::FeatureParticipantConfiguration()));

    auto a1 = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(R"(
Experimental:
  Management:
    ShareConfiguration: true
)"),
                                        "A1", registryUri + "/a");

    ASSERT_TRUE(state.WaitFor([&state] {
        return state.simulationNames == std::vector<std::string>{"a"}
               && state.participantsOfA == std::vector<std::string>{"A1"};
    }));
    {
        std::lock_guard<std::mutex> lock{state.mutex};
        EXPECT_THAT(state.createdSimulationNames, ElementsAre("a"));
    }

    // the RPC server of the management participant may not be discovered yet right after joining
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            state.hasReply = false;
        }
        ASSERT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(
                      client, "a", "A1", 2000000000, &state, &OnCParticipantConfiguration),
                  SilKit_ReturnCode_SUCCESS);
        ASSERT_TRUE(state.WaitFor([&state] { return state.hasReply; }));

        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (state.callStatus == SilKit_RpcCallStatus_Success)
            {
                break;
            }
        }
        std::this_thread::sleep_for(100ms);
    }

    {
        std::lock_guard<std::mutex> lock{state.mutex};
        EXPECT_EQ(state.callStatus, SilKit_RpcCallStatus_Success);
        EXPECT_EQ(state.status, SilKit_Experimental_ManagementParticipantConfigurationStatus_Available);
        EXPECT_THAT(state.configurationJson, HasSubstr("ShareConfiguration"));
    }

    EXPECT_EQ(SilKit_Experimental_ManagementClient_Destroy(clientGuard.release()), SilKit_ReturnCode_SUCCESS);
}

void SilKitCALL OnCUnansweredParticipantConfiguration(
    void* context, SilKit_Experimental_ManagementClient*, SilKit_RpcCallStatus callStatus,
    const SilKit_Experimental_ManagementParticipantConfigurationReply* reply)
{
    EXPECT_EQ(reply, nullptr);
    static_cast<UnansweredRequests*>(context)->OnResult(static_cast<RpcCallStatus>(callStatus));
}

TEST(ITest_Management, c_api_destroy_completes_pending_requests)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    // declared before the server and the client, which call into it until they are destroyed
    UnansweredRequests requests;
    auto server = CreateSilentServer(registryUri, requests);

    SilKit_ParticipantConfiguration* configuration{nullptr};
    ASSERT_EQ(SilKit_ParticipantConfiguration_FromString(&configuration, ""), SilKit_ReturnCode_SUCCESS);
    SilKit_Experimental_ManagementClient* client{nullptr};
    ASSERT_EQ(SilKit_Experimental_ManagementClient_Create(&client, configuration, registryUri.c_str()),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_ParticipantConfiguration_Destroy(configuration), SilKit_ReturnCode_SUCCESS);
    std::unique_ptr<SilKit_Experimental_ManagementClient, ClientDestroyer> clientGuard{client};

    ASSERT_TRUE(requests.SendUntilServerCalled([client, &requests] {
        const auto timeout = static_cast<SilKit_NanosecondsTime>(std::chrono::nanoseconds{60s}.count());
        EXPECT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(
                      client, "a", "A1", timeout, &requests, &OnCUnansweredParticipantConfiguration),
                  SilKit_ReturnCode_SUCCESS);
    }));

    size_t numResults{0};
    {
        std::lock_guard<std::mutex> lock{requests.mutex};
        numResults = requests.results.size();
    }

    EXPECT_EQ(SilKit_Experimental_ManagementClient_Destroy(clientGuard.release()), SilKit_ReturnCode_SUCCESS);

    std::lock_guard<std::mutex> lock{requests.mutex};
    ASSERT_EQ(requests.results.size(), numResults + 1);
    EXPECT_EQ(requests.results.back(), RpcCallStatus::UndefinedError);
}

} // namespace
