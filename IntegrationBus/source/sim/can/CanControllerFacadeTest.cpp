// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionComAdapter.hpp"

#include "CanControllerFacade.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::can;

class CanControllerFacadeTest : public testing::Test
{
public:
    CanControllerFacadeTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<ib::cfg::datatypes::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<ib::cfg::datatypes::ParticipantConfiguration>(ib::cfg::datatypes::ParticipantConfiguration());

    ib::cfg::datatypes::CanController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->canControllers.push_back(controllerNoNetworkCfg);

    ib::cfg::datatypes::CanController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->canControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(CanControllerFacadeTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<CanControllerFacade*>(comAdapter->CreateCanController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(CanControllerFacadeTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<CanControllerFacade*>(comAdapter->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(CanControllerFacadeTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<CanControllerFacade*>(comAdapter->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
