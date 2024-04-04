/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include "gtest/gtest.h"

#include "silkit/services/orchestration/string_utils.hpp"

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"
#include "InternalHelpers.hpp"

#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"

#include "CreateDashboard.hpp"

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Core;
using namespace SilKit::Config;
using namespace SilKit::Services;

class ITest_Dashboard : public ITest_DashboardTestHarness
{
protected:
    ITest_Dashboard()
        : ITest_DashboardTestHarness()
    {
    }

    ~ITest_Dashboard() {}

protected:
    void RunCanDemo(const std::string& participantName1, const std::string& participantName2,
                    const std::string& canonicalName, const std::string& networkName)
    {
        _simTestHarness->CreateSystemController();
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName1);
            auto&& participant = simParticipant->Participant();
            (void)participant->CreateCanController(canonicalName, networkName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            timeSyncService->SetSimulationStepHandler(
                CreateSimulationStepHandler(participantName1, lifecycleService),
                10ms);
        }
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName2);
            auto&& participant = simParticipant->Participant();
            (void)participant->CreateCanController(canonicalName, networkName);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
        _simTestHarness->ResetParticipants();
    }

    Orchestration::ITimeSyncService::SimulationStepHandler CreateSimulationStepHandler(
        const std::string& participantName, Orchestration::ILifecycleService* lifecycleService)
    {
        return [participantName, lifecycleService](auto now, auto /*duration*/) {
            if (now > 20ms)
            {
                Log() << participantName << ": stopping";
                lifecycleService->Stop("Coordinated Participant " + participantName + " ends test");
            }
        };
    }

    Orchestration::ISystemMonitor::ParticipantStatusHandler CreateAutonomousParticipantStatusHandler(
        const std::string& participantName, Orchestration::ILifecycleService* lifecycleService)
    {
        return [participantName, lifecycleService](auto status) {
            if (status.state == Orchestration::ParticipantState::Running && status.participantName == participantName)
            {
                Log() << participantName << ": stopping";
                lifecycleService->Stop("Autonomous Participant " + participantName + " ends test");
            }
        };
    }

    SilKit::Dashboard::TestResult CreateExpectedTestResult(
        std::vector<std::map<std::string, std::map<uint64_t, SilKit::Dashboard::Service>>> simulations, bool sync)
    {
        std::set<std::string> expectedStates{"servicescreated",
                                             "communicationinitializing",
                                             "communicationinitialized",
                                             "readytorun",
                                             "running",
                                             "stopping",
                                             "stopped"};
        SilKit::Dashboard::TestResult expected{};
        expected.objectCount = 0;
        for (auto simulationIdx = 0u; simulationIdx < simulations.size(); simulationIdx++)
        {
            std::map<std::string, std::map<uint64_t, SilKit::Dashboard::Service>> servicesByParticipant =
                simulations[simulationIdx];
            auto simulationId = simulationIdx + 1;
            if (sync)
            {
                expected.dataBySimulation[simulationId].participants.insert("SystemController");
                expected.dataBySimulation[simulationId].participants.insert("InternalSystemMonitor");
                expected.dataBySimulation[simulationId].statesByParticipant["InternalSystemMonitor"] = expectedStates;
            }
            for (auto i = servicesByParticipant.begin(); i != servicesByParticipant.end(); ++i)
            {
                expected.dataBySimulation[simulationId].participants.insert(i->first);
                expected.dataBySimulation[simulationId].statesByParticipant[i->first] = expectedStates;
            }
            expected.dataBySimulation[simulationId].servicesByParticipant = servicesByParticipant;
            if (sync)
            {
                expected.dataBySimulation[simulationId].systemStates = expectedStates;
            }
            expected.dataBySimulation[simulationId].stopped = true;
        }
        return expected;
    }
};

void CheckStates(std::set<std::string> actual, std::set<std::string> expected, const std::string& participantName,
                 uint64_t simulationId)
{
    ASSERT_GE(actual.size(), expected.size()) << "Wrong number of states for " << participantName << " of simulation "
                                              << simulationId << "!";
    for (auto&& state : expected)
    {
        ASSERT_TRUE(actual.find(state) != actual.end())
            << "State " << state << " for " << participantName << " of simulation " << simulationId << " not found!";
    }
}

void CheckMatchingLabel(MatchingLabel actual, MatchingLabel expected, const std::string& participantName,
                        uint64_t simulationId)
{
    ASSERT_EQ(actual.key, expected.key) << "Unexpected label key for " << participantName << " of simulation "
                                        << simulationId << "!";
    ASSERT_EQ(actual.value, expected.value)
        << "Unexpected label value for " << participantName << " of simulation " << simulationId << "!";
}

void CheckService(SilKit::Dashboard::Service actual, SilKit::Dashboard::Service expected,
                  const std::string& participantName, uint64_t simulationId)
{
    ASSERT_EQ(actual.parentServiceId, expected.parentServiceId)
        << "Unexpected parent service id for " << participantName << " of simulation " << simulationId << "!";
    ASSERT_EQ(actual.serviceType, expected.serviceType)
        << "Unexpected service type for " << participantName << " of simulation " << simulationId << "!";
    if (expected.serviceType == "datapublisher" || expected.serviceType == "datasubscriber")
    {
        ASSERT_EQ(actual.serviceName, expected.serviceName)
            << "Unexpected service name for " << participantName << " of simulation " << simulationId << "!";
        ASSERT_EQ(actual.spec.topic, expected.spec.topic)
            << "Unexpected topic for " << participantName << " of simulation " << simulationId << "!";
        for (auto i = 0u; i < expected.spec.labels.size(); i++)
        {
            CheckMatchingLabel(actual.spec.labels.at(i), expected.spec.labels.at(i), participantName, simulationId);
        }
        return;
    }
    if (expected.serviceType == "datasubscriberinternal")
    {
        return;
    }
    if (expected.serviceType == "rpcclient" || expected.serviceType == "rpcserver")
    {
        ASSERT_EQ(actual.serviceName, expected.serviceName)
            << "Unexpected service name for " << participantName << " of simulation " << simulationId << "!";
        ASSERT_EQ(actual.spec.functionName, expected.spec.functionName)
            << "Unexpected functionName for " << participantName << " of simulation " << simulationId << "!";
        for (auto i = 0u; i < expected.spec.labels.size(); i++)
        {
            CheckMatchingLabel(actual.spec.labels.at(i), expected.spec.labels.at(i), participantName, simulationId);
        }
        return;
    }
    if (expected.serviceType == "rpcserverinternal")
    {
        return;
    }
    ASSERT_EQ(actual.serviceName, expected.serviceName)
        << "Unexpected service name for " << participantName << " of simulation " << simulationId << "!";
    ASSERT_EQ(actual.networkName, expected.networkName)
        << "Unexpected network name for " << participantName << " of simulation " << simulationId << "!";
}

void CheckServices(std::map<uint64_t, SilKit::Dashboard::Service> actual,
                   std::map<uint64_t, SilKit::Dashboard::Service> expected, const std::string& participantName,
                   uint64_t simulationId)
{
    ASSERT_EQ(actual.size(), expected.size())
        << "Wrong number of services for " << participantName << " of simulation " << simulationId << "!";
    for (auto i = expected.begin(); i != expected.end(); ++i)
    {
        CheckService(actual[i->first], i->second, participantName, simulationId);
    }
}

void CheckLinks(std::set<SilKit::Dashboard::Link> actual, std::set<SilKit::Dashboard::Link> expected,
                uint64_t simulationId)
{
    ASSERT_EQ(actual.size(), expected.size()) << "Wrong number of links for simulation " << simulationId << "!";
    for (auto&& link : expected)
    {
        ASSERT_TRUE(actual.find(link) != actual.end())
            << "Link " << link.name << " " << link.type << " not found for simulation " << simulationId << "!";
    }
}

void CheckSimulationData(SilKit::Dashboard::SimulationData actual, SilKit::Dashboard::SimulationData expected,
                         uint64_t simulationId)
{
    ASSERT_EQ(actual.participants.size(), expected.participants.size())
        << "Unexpected participant count for simulation " << simulationId << "!";
    for (auto participantName : expected.participants)
    {
        ASSERT_TRUE(actual.participants.find(participantName) != actual.participants.end())
            << "Participant " << participantName << " should have been created for simulation " << simulationId << "!";
    }
    for (auto j = expected.statesByParticipant.begin(); j != expected.statesByParticipant.end(); ++j)
    {
        auto participantName = j->first;
        CheckStates(actual.statesByParticipant[participantName], expected.statesByParticipant[participantName],
                    participantName, simulationId);
    }
    for (auto j = expected.servicesByParticipant.begin(); j != expected.servicesByParticipant.end(); ++j)
    {
        auto participantName = j->first;
        CheckServices(actual.servicesByParticipant[participantName], expected.servicesByParticipant[participantName],
                      participantName, simulationId);
    }
    for (auto j = expected.linksByParticipant.begin(); j != expected.linksByParticipant.end(); ++j)
    {
        auto participantName = j->first;
        CheckLinks(actual.linksByParticipant[participantName], expected.linksByParticipant[participantName],
                   simulationId);
    }
    CheckStates(actual.systemStates, expected.systemStates, "system", simulationId);
    ASSERT_EQ(actual.stopped, expected.stopped) << "Simulation " << simulationId << " should have been stopped!";
}

void CheckTestResult(SilKit::Dashboard::TestResult actual, SilKit::Dashboard::TestResult expected)
{
    ASSERT_EQ(actual.errorStatus, expected.errorStatus) << "No error expected!";
    ASSERT_EQ(actual.errorMessage, expected.errorMessage) << "No error expected!";
    ASSERT_EQ(actual.objectCount, expected.objectCount) << "Oatpp should not leak objects!";
    ASSERT_EQ(actual.allSimulationsFinished, expected.allSimulationsFinished) << "All simulations should be finished!";
    ASSERT_EQ(actual.dataBySimulation.size(), expected.dataBySimulation.size()) << "Unexpected simulation count!";
    for (auto i = expected.dataBySimulation.begin(); i != expected.dataBySimulation.end(); ++i)
    {
        auto simulationId = i->first;
        ASSERT_EQ(actual.dataBySimulation.count(simulationId), 1) << "Simulation Ids differ!";
        CheckSimulationData(actual.dataBySimulation[simulationId], i->second, simulationId);
    }
}

TEST_F(ITest_Dashboard, dashboard_no_simulation)
{
    SetupFromParticipantLists({}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this]() {
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
        },
        0);
    _simTestHarness->ResetRegistry();
    CheckTestResult(testResult, CreateExpectedTestResult({}, false));
}

TEST_F(ITest_Dashboard, dashboard_unicode_emoji_coordinated)
{
    // the two participants use the car emoji (🚗) in their name, which is escaped for the dashboard
    const auto participantName1 = "\xf0\x9f\x9a\x97 CanReader";
    const auto expectedParticipantName1 = "%f0%9f%9a%97%20CanReader";
    const auto participantName2 = "\xf0\x9f\x9a\x97 CanWriter";
    const auto expectedParticipantName2 = "%f0%9f%9a%97%20CanWriter";
    const auto canonicalName = "CanController1";
    const auto networkName = "CAN1";
    SetupFromParticipantLists({participantName1, participantName2}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName1, &participantName2, &canonicalName, &networkName]() {
          RunCanDemo(participantName1, participantName2, canonicalName, networkName);
        });
    _simTestHarness->ResetRegistry();
    CheckTestResult(testResult,
                    CreateExpectedTestResult(
                        {{{expectedParticipantName1, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}},
                          {expectedParticipantName2, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}}}},
                        true));
}

TEST_F(ITest_Dashboard, dashboard_can_coordinated)
{
    const auto participantName1 = "CanReader";
    const auto participantName2 = "CanWriter";
    const auto canonicalName = "CanController1";
    const auto networkName = "CAN1";
    SetupFromParticipantLists({participantName1, participantName2}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName1, &participantName2, &canonicalName, &networkName]() {
            RunCanDemo(participantName1, participantName2, canonicalName, networkName);
        });
    _simTestHarness->ResetRegistry();
    CheckTestResult(
        testResult,
        CreateExpectedTestResult({{{participantName1, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}},
                                   {participantName2, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}}}},
                                 true));
}

TEST_F(ITest_Dashboard, dashboard_can_repeat)
{
    const auto participantName1 = "CanReader";
    const auto participantName2 = "CanWriter";
    const auto canonicalName = "CanController1";
    const auto networkName = "CAN1";
    SetupFromParticipantLists({participantName1, participantName2}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName1, &participantName2, &canonicalName, &networkName]() {
            RunCanDemo(participantName1, participantName2, canonicalName, networkName);
            RunCanDemo(participantName1, participantName2, canonicalName, networkName);
        },
        2);
    _simTestHarness->ResetRegistry();
    std::map<std::string, std::map<uint64_t, SilKit::Dashboard::Service>> simulation{
        {participantName1, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}},
        {participantName2, {{6, {"", "cancontroller", canonicalName, networkName, {}}}}}};
    CheckTestResult(testResult, CreateExpectedTestResult({simulation, simulation}, true));
}

TEST_F(ITest_Dashboard, dashboard_pubsub_mix)
{
    const auto participantName1 = "Publisher";
    const auto canonicalName1 = "PubCtrl";
    const auto topic{"Topic"};
    const auto mediaType{"A"};
    PubSub::PubSubSpec spec1{topic, mediaType};
    SilKit::Services::MatchingLabel label1{"Key", "Value", MatchingLabel::Kind::Optional};
    spec1.AddLabel(label1);
    const auto participantName2 = "Subscriber";
    const auto canonicalName2 = "SubCtrl";
    PubSub::PubSubSpec spec2{topic, mediaType};
    SilKit::Services::MatchingLabel label2{"Key", "Value", MatchingLabel::Kind::Mandatory};
    spec2.AddLabel(label2);
    SetupFromParticipantLists({participantName1}, {participantName2});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName1, &canonicalName1, &spec1, &participantName2, &canonicalName2, &spec2]() {
            _simTestHarness->CreateSystemController();
            {
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName1);
                auto&& participant = simParticipant->Participant();
                (void)participant->CreateDataPublisher(canonicalName1, spec1);
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
                timeSyncService->SetSimulationStepHandler(
                    CreateSimulationStepHandler(participantName1, lifecycleService), 10ms);
            }
            {
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName2);
                auto&& participant = simParticipant->Participant();
                (void)participant->CreateDataSubscriber(canonicalName2, spec2, [](auto, const auto&) {
                });
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& systemMonitor = simParticipant->GetOrCreateSystemMonitor();
                (void)systemMonitor->AddParticipantStatusHandler(
                    CreateAutonomousParticipantStatusHandler(participantName2, lifecycleService));
            }
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
            _simTestHarness->ResetParticipants();
        });
    _simTestHarness->ResetRegistry();
    CheckTestResult(
        testResult,
        CreateExpectedTestResult(
            {{{participantName1,
               {{6, {"", "datapublisher", canonicalName1, "IGNORED", {topic, "IGNORED", mediaType, {label1}}}}}},
              {participantName2,
               {{6, {"", "datasubscriber", canonicalName2, "IGNORED", {topic, "IGNORED", mediaType, {label2}}}},
                {7, {"6", "datasubscriberinternal", "IGNORED", "IGNORED", {}}}}}}},
            true));
}

TEST_F(ITest_Dashboard, dashboard_rpc_autonomous)
{
    const auto participantName1 = "Client";
    const auto canonicalName1 = "ClientCtrl";
    const auto functionName{"func"};
    const auto mediaType{"A"};
    Rpc::RpcSpec spec1{functionName, mediaType};
    SilKit::Services::MatchingLabel label1{"Key", "Value", MatchingLabel::Kind::Mandatory};
    spec1.AddLabel(label1);
    const auto participantName2 = "Server";
    const auto canonicalName2 = "ServerCtrl";
    Rpc::RpcSpec spec2{functionName, mediaType};
    SilKit::Services::MatchingLabel label2{"Key", "Value", MatchingLabel::Kind::Optional};
    spec2.AddLabel(label2);
    SetupFromParticipantLists({}, {participantName1, participantName2});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName1, &canonicalName1, &spec1, &participantName2, &canonicalName2, &spec2]() {
            {
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName1);
                auto&& participant = simParticipant->Participant();
                (void)participant->CreateRpcClient(canonicalName1, spec1, [](auto*, const auto&) {
                });
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& systemMonitor = simParticipant->GetOrCreateSystemMonitor();
                (void)systemMonitor->AddParticipantStatusHandler(
                    CreateAutonomousParticipantStatusHandler(participantName1, lifecycleService));
            }
            {
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName2);
                auto&& participant = simParticipant->Participant();
                (void)participant->CreateRpcServer(canonicalName2, spec2, [](auto*, const auto&) {
                });
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& systemMonitor = simParticipant->GetOrCreateSystemMonitor();
                (void)systemMonitor->AddParticipantStatusHandler(
                    CreateAutonomousParticipantStatusHandler(participantName2, lifecycleService));
            }
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
            _simTestHarness->ResetParticipants();
        });
    _simTestHarness->ResetRegistry();
    CheckTestResult(
        testResult,
        CreateExpectedTestResult(
            {{{participantName1,
               {{6, {"", "rpcclient", canonicalName1, "IGNORED", {"IGNORED", functionName, mediaType, {label1}}}}}},
              {participantName2,
               {{6, {"", "rpcserver", canonicalName2, "IGNORED", {"IGNORED", functionName, mediaType, {label2}}}},
                {7, {"6", "rpcserverinternal", "IGNORED", "IGNORED", {}}}}}}},
            false));
}

TEST_F(ITest_Dashboard, dashboard_netsim_coordinated)
{
    const auto participantName = "NetSim";
    SetupFromParticipantLists({participantName}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName]() {
            _simTestHarness->CreateSystemController();
            {
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName, _participantConfig);
                auto&& participant = simParticipant->Participant();
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
                auto&& participantInternal = &SilKit::Tests::ToParticipantInternal(*participant);
                auto&& serviceDiscovery = participantInternal->GetServiceDiscovery();

                lifecycleService->SetCommunicationReadyHandler([participantName, serviceDiscovery]() {
                    ServiceDescriptor linkDescriptor{};
                    linkDescriptor.SetServiceType(ServiceType::Link);
                    linkDescriptor.SetParticipantNameAndComputeId(participantName);
                    for (auto cfg :
                         {std::make_pair(NetworkType::CAN, "CAN1"), std::make_pair(NetworkType::Ethernet, "ETH1"),
                          std::make_pair(NetworkType::FlexRay, "FR1"), std::make_pair(NetworkType::LIN, "LIN1")})
                    {
                        linkDescriptor.SetNetworkType(cfg.first);
                        linkDescriptor.SetNetworkName(cfg.second);
                        linkDescriptor.SetServiceName(cfg.second);
                        serviceDiscovery->NotifyServiceCreated(std::move(linkDescriptor));
                    }
                });
                timeSyncService->SetSimulationStepHandler(
                    [simParticipant](auto currentSimTime, auto duration) {
                        if(currentSimTime > duration) {
                            Log() << simParticipant->Name() << ": stopping";
                            simParticipant->Stop();
                        }
                    },
                    10ms);
            }
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
            _simTestHarness->ResetParticipants();
        });
    _simTestHarness->ResetRegistry();
    auto expected = ITest_Dashboard::CreateExpectedTestResult({{{participantName, {}}}}, true);
    expected.dataBySimulation[1].linksByParticipant = {
        {participantName, {{"can", "CAN1"}, {"ethernet", "ETH1"}, {"flexray", "FR1"}, {"lin", "LIN1"}}}};
    CheckTestResult(testResult, expected);
}

} //end namespace
