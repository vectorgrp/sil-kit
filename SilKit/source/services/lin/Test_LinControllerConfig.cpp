// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "LinController.hpp"

namespace {
using namespace SilKit::Core;
using namespace SilKit::Services::Lin;

class Test_LinControllerConfig : public testing::Test
{
public:
    Test_LinControllerConfig(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<SilKit::Config::ParticipantConfiguration>(SilKit::Config::ParticipantConfiguration());

    SilKit::Config::LinController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->linControllers.push_back(controllerNoNetworkCfg);

    SilKit::Config::LinController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->linControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(Test_LinControllerConfig, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<LinController*>(participant->CreateLinController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(Test_LinControllerConfig, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<LinController*>(participant->CreateLinController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

} // anonymous namespace
