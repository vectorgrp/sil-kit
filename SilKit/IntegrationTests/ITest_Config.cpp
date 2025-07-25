// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT


#include <iostream>
#include <cstring>

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
    ASSERT_GT(jsonString.size(), 0u);
    auto&& config2 = SilKit::Config::ParticipantConfigurationFromString(jsonString);
    auto jsonString2 = SilKit::Config::ParticipantConfigurationToJson(config2);
    ASSERT_GT(jsonString2.size(), 0u);
    EXPECT_EQ(jsonString, jsonString2) << "The parsed config should be the same as the original yaml";
}

TEST(ITest_Config, test_to_json_in_capi)
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

    SilKit_ParticipantConfiguration* config{nullptr};
    auto result = SilKit_ParticipantConfiguration_FromString(&config, configString);
    ASSERT_EQ(result, SilKit_ReturnCode_SUCCESS);

    std::array<char, 1024> buf{};
    std::memset(buf.data(), 'A', buf.size());
    size_t size{};

    // get size
    result = SilKit_ParticipantConfiguration_ToJson(config, nullptr, &size);
    ASSERT_EQ(result, SilKit_ReturnCode_SUCCESS);
    ASSERT_GT(size, 0u) << "the string should have length > 0";
    ASSERT_LT(size, buf.size()) << "the string should have length <= buf.size()";

    //get contents, n
    auto&& ptr = buf.data();
    result = SilKit_ParticipantConfiguration_ToJson(config, &ptr, &size);
    ASSERT_EQ(result, SilKit_ReturnCode_SUCCESS);
    ASSERT_GT(size, 0u) << "the string should have length > 0";
    ASSERT_EQ(ptr[size], 'A') << "there should be no trailing \0";

    result = SilKit_ParticipantConfiguration_Destroy(config);
    ASSERT_EQ(result, SilKit_ReturnCode_SUCCESS);
}


} // anonymous namespace
