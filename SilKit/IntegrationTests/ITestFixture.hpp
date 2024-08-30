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
        , _dashboardParticipantConfig(R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Info"}]}})")
        , _participantConfig(R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Info"}]}})")
    {
    }
    ~ITest_DashboardTestHarness() {}

protected:
    void SetupFromParticipantLists(std::vector<std::string> coordinatedParticipantNames,
                                   std::vector<std::string> autonomousParticipantNames)
    {
        // create test harness with deferred participant and controller creation.
        // Will only create the SIL Kit Registry and tell the SystemController the participantNames
        _simTestHarness =
            std::make_unique<SimTestHarness>(coordinatedParticipantNames, "silkit://localhost:0", true, true,
                                                           autonomousParticipantNames);
        _registryUri = _simTestHarness->GetRegistryUri();
    }

protected: // members
    std::string _dashboardUri;
    std::string _dashboardParticipantConfig;
    std::string _participantConfig;
};

} //namespace Tests
} //namespace SilKit
