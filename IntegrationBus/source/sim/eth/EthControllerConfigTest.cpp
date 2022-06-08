// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionParticipant.hpp"

#include "EthController.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::eth;

class EthernetControllerConfigTest : public testing::Test
{
public:
    EthernetControllerConfigTest(){};
};

auto PrepareParticipantConfiguration() -> std::shared_ptr<ib::cfg::ParticipantConfiguration>
{
    auto mockConfig =
        std::make_shared<ib::cfg::ParticipantConfiguration>(ib::cfg::ParticipantConfiguration());

    ib::cfg::EthernetController controllerNoNetworkCfg;
    controllerNoNetworkCfg.name = "ControllerWithoutNetwork";
    mockConfig->ethernetControllers.push_back(controllerNoNetworkCfg);

    ib::cfg::EthernetController controllerWithNetworkCfg;
    controllerWithNetworkCfg.name = "ControllerWithNetwork";
    controllerWithNetworkCfg.network = "ConfigNet";
    mockConfig->ethernetControllers.push_back(controllerWithNetworkCfg);

    return mockConfig;
}

TEST(EthernetControllerConfigTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthController*>(participant->CreateEthernetController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(EthernetControllerConfigTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthController*>(participant->CreateEthernetController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(EthernetControllerConfigTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto participant = ib::mw::CreateNullConnectionParticipantImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthController*>(participant->CreateEthernetController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
