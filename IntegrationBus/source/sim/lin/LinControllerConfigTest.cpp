// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "LinController.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::lin;

class LinControllerConfigTest : public testing::Test
{
public:
    LinControllerConfigTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<ib::cfg::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<ib::cfg::ParticipantConfiguration>(ib::cfg::ParticipantConfiguration());

    ib::cfg::LinController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->linControllers.push_back(controllerNoNetworkCfg);

    ib::cfg::LinController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->linControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(LinControllerConfigTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<LinController*>(participant->CreateLinController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(LinControllerConfigTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller = dynamic_cast<LinController*>(participant->CreateLinController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(LinControllerConfigTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller = dynamic_cast<LinController*>(participant->CreateLinController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
