// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "CanController.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::can;

class CanControllerConfigTest : public testing::Test
{
public:
    CanControllerConfigTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<ib::cfg::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<ib::cfg::ParticipantConfiguration>(ib::cfg::ParticipantConfiguration());

    ib::cfg::CanController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->canControllers.push_back(controllerNoNetworkCfg);

    ib::cfg::CanController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->canControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(CanControllerConfigTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller =
        dynamic_cast<CanController*>(participant->CreateCanController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(CanControllerConfigTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<CanController*>(participant->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(CanControllerConfigTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant");

    auto controller = dynamic_cast<CanController*>(participant->CreateCanController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
