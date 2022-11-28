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

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"

#include "silkit/services/all.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "silkit/config/all.hpp"

#include "CreateDashboard.hpp"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Core;
using namespace SilKit::Config;
using namespace SilKit::Services;

class DashboardTestHarness : public ITest_SimTestHarness
{
protected:
    DashboardTestHarness()
        : ITest_SimTestHarness()
        , _dashboardUri(MakeTestDashboardUri())
        , _participantConfig(
              ParticipantConfigurationFromString(R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Info"}]}})"))
    {
    }
    ~DashboardTestHarness() {}

protected:
    Orchestration::ITimeSyncService::SimulationStepHandler CreateSimulationStepHandler(
        const std::string& participantName, Orchestration::ILifecycleService* lifecycleService)
    {
        return [this, participantName, lifecycleService](auto now, auto) {
            if (now < 200ms)
            {
                return;
            }
            if (!_result)
            {
                _result = true;
                lifecycleService->Stop("Test done");
                Log() << "---   " << participantName << ": Sending Stop from";
            }
        };
    }

    SilKit::Dashboard::TestResult CreateExpectedTestResult(
        std::map<std::string, std::set<SilKit::Dashboard::Service>> servicesByParticipant)
    {
        std::set<std::string> expectedStates{"servicescreated",
                                             "communicationinitializing",
                                             "communicationinitialized",
                                             "readytorun",
                                             "running",
                                             "stopping",
                                             "stopped",
                                             "shuttingdown",
                                             "shutdown"};
        SilKit::Dashboard::TestResult expected{};
        expected.objectCount = 0;
        expected.dataBySimulation[1].participants = {"InternalSystemMonitor"};
        expected.dataBySimulation[1].statesByParticipant = {{"InternalSystemMonitor", expectedStates}};
        for (auto i = servicesByParticipant.begin(); i != servicesByParticipant.end(); ++i)
        {
            expected.dataBySimulation[1].participants.insert(i->first);
            expected.dataBySimulation[1].statesByParticipant[i->first] = expectedStates;
        }
        expected.dataBySimulation[1].servicesByParticipant = servicesByParticipant;
        expected.dataBySimulation[1].links = {};
        expected.dataBySimulation[1].systemStates = expectedStates;
        expected.dataBySimulation[1].stopped = true;
        return expected;
    }

protected: // members
    std::string _dashboardUri;
    std::shared_ptr<IParticipantConfiguration> _participantConfig;
    bool _result{false};
};

void CheckStates(std::set<std::string> actual, std::set<std::string> expected, const std::string& participantName)
{
    ASSERT_EQ(actual.size(), expected.size()) << "Wrong number of states for " << participantName << "!";
    for (auto&& state : expected)
    {
        ASSERT_TRUE(actual.find(state) != actual.end())
            << "State " << state << " for " << participantName << " not found!";
    }
}

void CheckMatchingLabel(MatchingLabel actual, MatchingLabel expected, const std::string& participantName)
{
    ASSERT_EQ(actual.key, expected.key) << "Unexpected label key for " << participantName << "!";
    ASSERT_EQ(actual.value, expected.value) << "Unexpected label value for " << participantName << "!";
}

void CheckService(SilKit::Dashboard::Service actual, SilKit::Dashboard::Service expected,
                  const std::string& participantName)
{
    if (expected.serviceType == "datapublisher")
    {
        ASSERT_EQ(actual.serviceType, expected.serviceType)
            << "Unexpected controller type for " << participantName << "!";
        ASSERT_EQ(actual.serviceName, expected.serviceName) << "Unexpected service name for " << participantName << "!";
        ASSERT_EQ(actual.topic, expected.topic) << "Unexpected topic for " << participantName << "!";
        for (auto i = 0u; i < expected.labels.size(); i++)
        {
            CheckMatchingLabel(actual.labels.at(i), expected.labels.at(i), participantName);
        }
        return;
    }
    if (expected.serviceType == "datasubscriber")
    {
        ASSERT_EQ(actual.serviceType, expected.serviceType)
            << "Unexpected controller type for " << participantName << "!";
        return;
    }
    if (expected.serviceType == "rpcclient")
    {
        ASSERT_EQ(actual.serviceType, expected.serviceType)
            << "Unexpected controller type for " << participantName << "!";
        ASSERT_EQ(actual.serviceName, expected.serviceName) << "Unexpected service name for " << participantName << "!";
        ASSERT_EQ(actual.functionName, expected.functionName)
            << "Unexpected functionName for " << participantName << "!";
        for (auto i = 0u; i < expected.labels.size(); i++)
        {
            CheckMatchingLabel(actual.labels.at(i), expected.labels.at(i), participantName);
        }
        return;
    }
    if (expected.serviceType == "rpcserver")
    {
        ASSERT_EQ(actual.serviceType, expected.serviceType)
            << "Unexpected controller type for " << participantName << "!";
        return;
    }
    ASSERT_EQ(actual.serviceType, expected.serviceType) << "Unexpected controller type for " << participantName << "!";
    ASSERT_EQ(actual.serviceName, expected.serviceName) << "Unexpected service name for " << participantName << "!";
    ASSERT_EQ(actual.networkName, expected.networkName) << "Unexpected network name for " << participantName << "!";
}

void CheckServices(std::set<SilKit::Dashboard::Service> actual, std::set<SilKit::Dashboard::Service> expected,
                   const std::string& participantName)
{
    ASSERT_EQ(actual.size(), expected.size()) << "Wrong number of services for " << participantName << "!";
    for (auto&& service : expected)
    {
        ASSERT_TRUE(actual.find(service) != actual.end())
            << "Service " << service.serviceName << " " << service.serviceType << " not found!";
        CheckService(*(actual.find(service)), service, participantName);
    }
}

void CheckLinks(std::set<SilKit::Dashboard::Link> actual, std::set<SilKit::Dashboard::Link> expected)
{
    ASSERT_EQ(actual.size(), expected.size()) << "Wrong number of links!";
    for (auto&& link : expected)
    {
        ASSERT_TRUE(actual.find(link) != actual.end()) << "Link " << link.name << " " << link.type << " not found!";
    }
}

void CheckSimulationData(SilKit::Dashboard::SimulationData actual, SilKit::Dashboard::SimulationData expected)
{
    ASSERT_EQ(actual.participants.size(), expected.participants.size()) << "Unexpected participant count!";
    for (auto participantName : expected.participants)
    {
        ASSERT_TRUE(actual.participants.find(participantName) != actual.participants.end())
            << "Participant " << participantName << " should have been created!";
    }
    for (auto j = expected.statesByParticipant.begin(); j != expected.statesByParticipant.end(); ++j)
    {
        auto participantName = j->first;
        CheckStates(actual.statesByParticipant[participantName], expected.statesByParticipant[participantName],
                    participantName);
    }
    for (auto j = expected.servicesByParticipant.begin(); j != expected.servicesByParticipant.end(); ++j)
    {
        auto participantName = j->first;
        CheckServices(actual.servicesByParticipant[participantName], expected.servicesByParticipant[participantName],
                      participantName);
    }
    CheckLinks(actual.links, expected.links);
    CheckStates(actual.systemStates, expected.systemStates, "system");
    ASSERT_EQ(actual.stopped, expected.stopped) << "Wrong simulation state!";
}

void CheckTestResult(SilKit::Dashboard::TestResult actual, SilKit::Dashboard::TestResult expected)
{
    ASSERT_EQ(actual.errorStatus, expected.errorStatus) << "No error expected!";
    ASSERT_EQ(actual.errorMessage, expected.errorMessage) << "No error expected!";
    ASSERT_EQ(actual.objectCount, expected.objectCount) << "Oatpp should not leak objects!";
    ASSERT_EQ(actual.dataBySimulation.size(), expected.dataBySimulation.size()) << "Unexpected simulation count!";
    for (auto i = expected.dataBySimulation.begin(); i != expected.dataBySimulation.end(); ++i)
    {
        auto simulationId = i->first;
        ASSERT_EQ(actual.dataBySimulation.count(simulationId), 1) << "Simulation Ids differ!";
        CheckSimulationData(actual.dataBySimulation[simulationId], i->second);
    }
}

TEST_F(DashboardTestHarness, dashboard_can)
{
    SetupFromParticipantList({"CanWriter"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        {
            /////////////////////////////////////////////////////////////////////////
            // CanWriter
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "CanWriter";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateCanController("CanController1", "CAN1");

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    SilKit::Dashboard::Service expectedService = {"cancontroller", "CanController1", "CAN1", "IGNORED", "IGNORED", {}};
    CheckTestResult(testResult, CreateExpectedTestResult({{"CanWriter", {expectedService}}}));
}

TEST_F(DashboardTestHarness, dashboard_ethernet)
{
    SetupFromParticipantList({"EthernetWriter"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        {
            /////////////////////////////////////////////////////////////////////////
            // EthernetWriter
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "EthernetWriter";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateEthernetController("EthernetController1", "ETH1");

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    SilKit::Dashboard::Service expectedService = {
        "ethernetcontroller", "EthernetController1", "ETH1", "IGNORED", "IGNORED", {}};
    CheckTestResult(testResult, CreateExpectedTestResult({{"EthernetWriter", {expectedService}}}));
}

TEST_F(DashboardTestHarness, dashboard_flexray)
{
    SetupFromParticipantList({"Node"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        {
            /////////////////////////////////////////////////////////////////////////
            // Node0
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "Node";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateFlexrayController("FlexrayController1", "FR1");

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    SilKit::Dashboard::Service expectedService = {
        "flexraycontroller", "FlexrayController1", "FR1", "IGNORED", "IGNORED", {}};
    CheckTestResult(testResult, CreateExpectedTestResult({{"Node", {expectedService}}}));
}

TEST_F(DashboardTestHarness, dashboard_lin)
{
    SetupFromParticipantList({"LinMaster"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        {
            /////////////////////////////////////////////////////////////////////////
            // LinMaster
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "LinMaster";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateLinController("LinController1", "LIN1");

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    SilKit::Dashboard::Service expectedService = {"lincontroller", "LinController1", "LIN1", "IGNORED", "IGNORED", {}};
    CheckTestResult(testResult, CreateExpectedTestResult({{"LinMaster", {expectedService}}}));
}

TEST_F(DashboardTestHarness, dashboard_pubsub)
{
    SetupFromParticipantList({"Publisher", "Subscriber"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        PubSub::PubSubSpec dataSpec{"Topic", "A"};
        dataSpec.AddLabel({"Key", "Value", MatchingLabel::Kind::Mandatory});
        {
            /////////////////////////////////////////////////////////////////////////
            // Publisher
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "Publisher";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            (void)simParticipant->GetOrCreateLifecycleService();
            (void)simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateDataPublisher("PubCtrl", dataSpec, 1);
        }
        {
            /////////////////////////////////////////////////////////////////////////
            // Subscriber
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "Subscriber";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateDataSubscriber("SubCtrl", dataSpec, [](auto, const auto&) {
            });

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    CheckTestResult(testResult,
                    CreateExpectedTestResult(
                        {{"Publisher",
                          {{"datapublisher",
                            "PubCtrl",
                            "IGNORED",
                            "Topic",
                            "IGNORED",
                            {{"Key", "Value", MatchingLabel::Kind::Mandatory}}}}},
                         {"Subscriber", {{"datasubscriber", "IGNORED", "IGNORED", "IGNORED", "IGNORED", {}}}}}));
}

TEST_F(DashboardTestHarness, dashboard_rpc)
{
    SetupFromParticipantList({"Client", "Server"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        Rpc::RpcSpec dataSpec{"func", "A"};
        dataSpec.AddLabel({"Key", "Value", MatchingLabel::Kind::Mandatory});
        {
            /////////////////////////////////////////////////////////////////////////
            // Client
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "Client";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            (void)simParticipant->GetOrCreateLifecycleService();
            (void)simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateRpcClient("ClientCtrl", dataSpec, [](auto, const auto&) {
            });
        }
        {
            /////////////////////////////////////////////////////////////////////////
            // Server
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "Server";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateRpcServer("ServerCtrl", dataSpec, [](auto, const auto&) {
            });

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    CheckTestResult(testResult, CreateExpectedTestResult(
                                    {{"Client",
                                      {{"rpcclient",
                                        "ClientCtrl",
                                        "IGNORED",
                                        "IGNORED",
                                        "func",
                                        {{"Key", "Value", MatchingLabel::Kind::Mandatory}}}}},
                                     {"Server", {{"rpcserver", "IGNORED", "IGNORED", "IGNORED", "IGNORED", {}}}}}));
}

TEST_F(DashboardTestHarness, dashboard_netsim)
{
    SetupFromParticipantList({"NetSim"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(_participantConfig, _registryUri, _dashboardUri, [this]() {
        {
            /////////////////////////////////////////////////////////////////////////
            // NetSim
            /////////////////////////////////////////////////////////////////////////
            const auto participantName = "NetSim";
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto* participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            auto&& participantInternal = dynamic_cast<IParticipantInternal*>(participant);
            auto&& serviceDiscovery = participantInternal->GetServiceDiscovery();

            lifecycleService->SetCommunicationReadyHandler([participantName, serviceDiscovery]() {
                ServiceDescriptor linkDescriptor{};
                linkDescriptor.SetServiceType(ServiceType::Link);
                linkDescriptor.SetParticipantName(participantName);
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

            timeSyncService->SetSimulationStepHandler(CreateSimulationStepHandler(participantName, lifecycleService),
                                                      1ms);
        }

        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    });
    auto expected = DashboardTestHarness::CreateExpectedTestResult({{"NetSim", {}}});
    expected.dataBySimulation[1].links = {{"can", "CAN1"}, {"ethernet", "ETH1"}, {"flexray", "FR1"}, {"lin", "LIN1"}};
    CheckTestResult(testResult, expected);
}

} //end namespace
