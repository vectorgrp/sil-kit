// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionComAdapter.hpp"

#include "FrControllerFacade.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::fr;

class FlexRayControllerFacadeTest : public testing::Test
{
public:
    FlexRayControllerFacadeTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<ib::cfg::datatypes::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<ib::cfg::datatypes::ParticipantConfiguration>(ib::cfg::datatypes::ParticipantConfiguration());

    ib::cfg::datatypes::FlexRayController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->flexRayControllers.push_back(controllerNoNetworkCfg);

    ib::cfg::datatypes::FlexRayController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->flexRayControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(FlexRayControllerFacadeTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller = dynamic_cast<FrControllerFacade*>(comAdapter->CreateFlexrayController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(FlexRayControllerFacadeTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<FrControllerFacade*>(comAdapter->CreateFlexrayController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(FlexRayControllerFacadeTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<FrControllerFacade*>(comAdapter->CreateFlexrayController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
