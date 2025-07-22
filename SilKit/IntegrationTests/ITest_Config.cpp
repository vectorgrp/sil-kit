// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT


#include <iostream>

#include "silkit/services/all.hpp"
#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace {

using namespace std::chrono_literals;
using namespace SilKit;


TEST(ITest_Config, test_to_json)
{
    auto&& configString = R"(schemaVersion: 1
Description: test

Middleware:
  RegistryUri: silkit://localhost:8501
  ConnectAttempts: 1
  TcpNoDelay: true
  AcceptorUris: [tcp://0.0.0.0:8502]
  RegistryAsFallbackProxy: true

Logging:
  Sinks:
    - Type: Stdout
      Level: Debug
)";

    auto&& config = SilKit::Config::ParticipantConfigurationFromString(configString);
    auto jsonString = SilKit::Config::ParticipantConfigurationToJson(config);
    auto&& config2 = SilKit::Config::ParticipantConfigurationFromString(jsonString);
    auto jsonString2 = SilKit::Config::ParticipantConfigurationToJson(config2);
    EXPECT_EQ(jsonString, jsonString2) << "The parsed config should be the same as the original yaml";
}

} // anonymous namespace
