// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "FlexrayController.hpp"

namespace {
using namespace SilKit::Core;
using namespace SilKit::Services::Flexray;

class FlexrayControllerConfigTest : public testing::Test
{
public:
    FlexrayControllerConfigTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<SilKit::Config::ParticipantConfiguration>(SilKit::Config::ParticipantConfiguration());

    SilKit::Config::FlexrayController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->flexrayControllers.push_back(controllerNoNetworkCfg);

    SilKit::Config::FlexrayController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->flexrayControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(FlexrayControllerConfigTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller =
        dynamic_cast<FlexrayController*>(participant->CreateFlexrayController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(FlexrayControllerConfigTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = SilKit::Core::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller =
        dynamic_cast<FlexrayController*>(participant->CreateFlexrayController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
