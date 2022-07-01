// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"
#include "ServiceDiscovery.hpp"
#include "ConfigurationTestUtils.hpp"
#include "VAsioRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;

class ServiceDiscoveryITest : public testing::Test
{
protected:

    ServiceDiscoveryITest()
    {
    }

};

// Tests that the service discovery handler fires for created services
// All created should be removed as well if a participant leaves
TEST_F(ServiceDiscoveryITest, discover_services)
{
    auto registryUri = MakeTestRegistryUri();
    size_t numberOfServices = 5;
    std::string subscriberName = "Subscriber";
    std::string publisherName = "Publisher";

    // Registry
    auto registry = std::make_unique<VAsioRegistry>(ib::cfg::MakeEmptyParticipantConfiguration());
    registry->ProvideDomain(registryUri);

    // Publisher that will leave the simulation and trigger service removal
    auto&& publisher =  ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), publisherName, registryUri);

    // Subscriber that monitors the services
    auto&& subscriber =
        ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), subscriberName, registryUri);

    // Services
    for (auto i = 0u; i < numberOfServices; i++)
    {
        const auto topic = "TopicName-" + std::to_string(i);
        const auto pubControllerName = "PubCtrl" + std::to_string(i);
        const auto subControllerName = "SubCtrl" + std::to_string(i);
        publisher->CreateDataPublisher(pubControllerName, topic, {}, {}, 0);
        subscriber->CreateDataSubscriber(subControllerName, topic, {}, {}, nullptr);
    }

    std::vector<std::string> createdServiceNames;
    std::vector<std::string> removedServiceNames;

    // Cast to internal participant for access to service discovery
    auto subscriberServiceDiscovery = dynamic_cast<IParticipantInternal*>(subscriber.get())->GetServiceDiscovery();
    
    auto allCreated = std::promise<void>();
    auto allRemoved = std::promise<void>();
    // Participants are already there, so the registration will trigger the provided handler immediately 
    subscriberServiceDiscovery->RegisterServiceDiscoveryHandler(
        [numberOfServices, &allRemoved, &allCreated, &createdServiceNames, &removedServiceNames, publisherName](
            auto discoveryType, const auto& service)
        {
            switch (discoveryType)
            {
            case ib::mw::service::ServiceDiscoveryEvent::Type::Invalid:
                break;
            case ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated:
                if (service.GetParticipantName() == publisherName)
                {
                    createdServiceNames.push_back(service.GetServiceName());
                    if (createdServiceNames.size() == numberOfServices)
                    {
                        allCreated.set_value();
                    }
                }
                break;
            case ib::mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved:
                if (service.GetParticipantName() == publisherName)
                {
                    removedServiceNames.push_back(service.GetServiceName());
                    if (removedServiceNames.size() == createdServiceNames.size())
                    {
                        allRemoved.set_value();
                    }
                }
                break;
            default:
                break;
            }
        });

    // Await the creation
    allCreated.get_future().wait_for(10s);

    // Kill the publisher
    publisher.reset();

    // Await the removal
    allRemoved.get_future().wait_for(10s);

    // At least the DataPublisher services got discovered (more could come in future, so check via >=)
    ASSERT_TRUE(createdServiceNames.size() >= numberOfServices);

    // All that got discovered should be removed as well
    std::sort(createdServiceNames.begin(), createdServiceNames.end());
    std::sort(removedServiceNames.begin(), removedServiceNames.end());
    EXPECT_EQ(createdServiceNames, removedServiceNames);
}

// Tests that the specific service discovery handler fires for created services
// All created should be removed as well if a participant leaves
TEST_F(ServiceDiscoveryITest, discover_specific_services)
{
    auto registryUri = MakeTestRegistryUri();
    size_t numberOfServices = 5;
    std::string subscriberName = "Subscriber";
    std::string publisherName = "Publisher";

    // Registry
    auto registry = std::make_unique<VAsioRegistry>(ib::cfg::MakeEmptyParticipantConfiguration());
    registry->ProvideDomain(registryUri);

    // Publisher that will leave the simulation and trigger service removal
    auto&& publisher = ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), publisherName, registryUri);

    // Subscriber that monitors the services
    auto&& subscriber = ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), subscriberName, registryUri);

    // Services
    const auto topic = "Topic";
    for (auto i = 0u; i < numberOfServices; i++)
    {
        const auto pubControllerName = "PubCtrl" + std::to_string(i);
        publisher->CreateDataPublisher(pubControllerName, topic, {}, {}, 0);
    }

    std::vector<std::string> createdServiceNames;
    std::vector<std::string> removedServiceNames;

    // Cast to internal participant for access to service discovery
    auto subscriberServiceDiscovery = dynamic_cast<IParticipantInternal*>(subscriber.get())->GetServiceDiscovery();

    auto allCreated = std::promise<void>();
    auto allRemoved = std::promise<void>();
    // Participants are already there, so the registration will trigger the provided handler immediately
    subscriberServiceDiscovery->RegisterSpecificServiceDiscoveryHandler(
        [numberOfServices, &allRemoved, &allCreated, &createdServiceNames, &removedServiceNames, publisherName](
            auto discoveryType, const auto& service) {
            switch (discoveryType)
            {
            case ib::mw::service::ServiceDiscoveryEvent::Type::Invalid: break;
            case ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated:
                if (service.GetParticipantName() == publisherName)
                {
                    createdServiceNames.push_back(service.GetServiceName());
                    if (createdServiceNames.size() == numberOfServices)
                    {
                        allCreated.set_value();
                    }
                }
                break;
            case ib::mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved:
                if (service.GetParticipantName() == publisherName)
                {
                    removedServiceNames.push_back(service.GetServiceName());
                    if (removedServiceNames.size() == createdServiceNames.size())
                    {
                        allRemoved.set_value();
                    }
                }
                break;
            default: break;
            }
        }, ib::mw::service::controllerTypeDataPublisher, topic);

    // Await the creation
    allCreated.get_future().wait_for(10s);

    // Kill the publisher
    publisher.reset();

    // Await the removal
    allRemoved.get_future().wait_for(10s);

    // The DataPublisher services get discovered by the specific handler
    ASSERT_TRUE(createdServiceNames.size() == numberOfServices);

    // All that got discovered should be removed as well
    std::sort(createdServiceNames.begin(), createdServiceNames.end());
    std::sort(removedServiceNames.begin(), removedServiceNames.end());
    EXPECT_EQ(createdServiceNames, removedServiceNames);
}


} // anonymous namespace
