// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "NullConnectionComAdapter.hpp"

#include "EthControllerFacade.hpp"

namespace {
using namespace ib::mw;
using namespace ib::sim::eth;

class EthernetControllerFacadeTest : public testing::Test
{
public:
    EthernetControllerFacadeTest(){};
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

TEST(EthernetControllerFacadeTest, create_controller_unconfigured)
{
    auto controllerName = "Controller";
    auto expectedNetworkName = "Controller";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthControllerFacade*>(comAdapter->CreateEthController(controllerName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(EthernetControllerFacadeTest, create_controller_configured_no_network)
{
    auto controllerName = "ControllerWithoutNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "TestNetwork";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthControllerFacade*>(comAdapter->CreateEthController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

TEST(EthernetControllerFacadeTest, create_controller_configured_with_network)
{
    auto controllerName = "ControllerWithNetwork";
    auto networkName = "TestNetwork";
    auto expectedNetworkName = "ConfigNet";

    auto&& config = PrepareParticipantConfiguration();

    auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(config, "TestParticipant", false);

    auto controller =
        dynamic_cast<EthControllerFacade*>(comAdapter->CreateEthController(controllerName, networkName));
    auto serviceDescr = controller->GetServiceDescriptor();
    EXPECT_EQ(serviceDescr.GetServiceName(), controllerName);
    EXPECT_EQ(serviceDescr.GetNetworkName(), expectedNetworkName);
}

}  // anonymous namespace
