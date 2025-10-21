// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "CanController.hpp"

namespace {
using namespace SilKit::Core;
using namespace SilKit::Services::Can;

class Test_CanControllerConfig : public testing::Test
{
public:
    Test_CanControllerConfig(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<SilKit::Config::ParticipantConfiguration>(SilKit::Config::ParticipantConfiguration());

    SilKit::Config::CanController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->canControllers.push_back(controllerNoNetworkCfg);

    SilKit::Config::CanController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->canControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(Test_CanControllerConfig, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<CanController*>(participant->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(Test_CanControllerConfig, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<CanController*>(participant->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

} // anonymous namespace
