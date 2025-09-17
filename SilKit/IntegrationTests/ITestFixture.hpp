// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <memory>
#include <future>
// for thread safe logging:
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>

#include "GetTestPid.hpp"
#include "SimTestHarness.hpp"

#include "gtest/gtest.h"

namespace SilKit {
namespace Tests {

class ITest_SimTestHarness : public testing::Test
{
protected: //CTor and operators
    auto TestHarness() -> SimTestHarness&
    {
        return *_simTestHarness;
    }

    void SetupFromParticipantList(std::vector<std::string> participantNames)
    {
        // create test harness with deferred participant creation.
        // Will only create the SIL Kit Registry and tell the SystemController the participantNames
        _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", true);
        _registryUri = _simTestHarness->GetRegistryUri();
    }

protected: // members
    std::string _registryUri{};
    std::unique_ptr<SimTestHarness> _simTestHarness;
};

class ITest_DashboardTestHarness : public ITest_SimTestHarness
{
protected:
    ITest_DashboardTestHarness()
        : ITest_SimTestHarness()
        , _dashboardUri(MakeTestDashboardUri())
    {
    }
    ~ITest_DashboardTestHarness() {}

protected:
    void SetupFromParticipantLists(std::vector<std::string> coordinatedParticipantNames,
                                   std::vector<std::string> autonomousParticipantNames)
    {
        // create test harness with deferred participant and controller creation.
        // Will only create the SIL Kit Registry and tell the SystemController the participantNames
        SimTestHarnessArgs args{};
        args.syncParticipantNames = std::move(coordinatedParticipantNames);
        args.asyncParticipantNames = std::move(autonomousParticipantNames);
        args.registry.participantConfiguration = _registryParticipantConfig;
        args.registry.listenUri = "silkit://localhost:0";
        args.deferParticipantCreation = true;
        args.deferSystemControllerCreation = true;

        _simTestHarness = std::make_unique<SimTestHarness>(args);
        _registryUri = _simTestHarness->GetRegistryUri();
    }

protected: // members
    std::string _dashboardUri;
    std::string _dashboardParticipantConfig = R"(
Logging:
  Sinks:
  - Type: Stdout
    Level: Info
)";
    std::string _participantConfig = R"(
Logging:
  Sinks:
  - Type: Stdout
    Level: Info
)";
    const std::string _registryParticipantConfig = R"(
Logging:
  Sinks:
  - Type: Stdout
    Level: Info
Experimental:
  Metrics:
    CollectFromRemote: true
)";

    std::string _registryConfiguration = "";
};

} //namespace Tests
} //namespace SilKit
