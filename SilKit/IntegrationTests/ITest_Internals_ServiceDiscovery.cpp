/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>

#include "silkit/services/all.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "functional.hpp"

#include "SimTestHarness.hpp"
#include "ServiceDiscovery.hpp"
#include "ConfigurationTestUtils.hpp"
#include "VAsioRegistry.hpp"
#include "CreateParticipantImpl.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;

class ITest_Internals_ServiceDiscovery : public testing::Test
{
protected:
    ITest_Internals_ServiceDiscovery() {}
};

// Tests that the service discovery handler fires for created services
// All created should be removed as well if a participant leaves
TEST_F(ITest_Internals_ServiceDiscovery, discover_services)
{
    size_t numberOfServices = 5;
    std::string subscriberName = "Subscriber";
    std::string publisherName = "Publisher";

    // Registry
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // Publisher that will leave the simulation and trigger service removal
    auto&& publisher = SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(),
                                                     publisherName, registryUri);

    // Subscriber that monitors the services
    auto&& subscriber = SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(),
                                                      subscriberName, registryUri);

    // Services
    for (auto i = 0u; i < numberOfServices; i++)
    {
        const auto topic = "TopicName-" + std::to_string(i);
        const auto pubControllerName = "PubCtrl" + std::to_string(i);
        const auto subControllerName = "SubCtrl" + std::to_string(i);
        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, {}};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, {}};
        publisher->CreateDataPublisher(pubControllerName, dataSpec, 0);
        subscriber->CreateDataSubscriber(subControllerName, matchingDataSpec, nullptr);
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
            auto discoveryType, const auto& service) {
        switch (discoveryType)
        {
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::Invalid:
            break;
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated:
            if (service.GetParticipantName() == publisherName)
            {
                createdServiceNames.push_back(service.GetServiceName());
                if (createdServiceNames.size() == numberOfServices)
                {
                    allCreated.set_value();
                }
            }
            break;
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved:
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
TEST_F(ITest_Internals_ServiceDiscovery, discover_specific_services)
{
    size_t numberOfServices = 5;
    std::string subscriberName = "Subscriber";
    std::string publisherName = "Publisher";

    // Registry
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // Publisher that will leave the simulation and trigger service removal
    auto&& publisher = SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(),
                                                     publisherName, registryUri);

    // Subscriber that monitors the services
    auto&& subscriber = SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(),
                                                      subscriberName, registryUri);

    // Services
    const std::string topic = "Topic";
    for (auto i = 0u; i < numberOfServices; i++)
    {
        const auto pubControllerName = "PubCtrl" + std::to_string(i);
        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, {}};
        publisher->CreateDataPublisher(pubControllerName, dataSpec, 0);
    }

    std::vector<std::string> createdServiceNames;
    std::vector<std::string> removedServiceNames;

    // Cast to internal participant for access to service discovery
    auto subscriberServiceDiscovery = dynamic_cast<IParticipantInternal*>(subscriber.get())->GetServiceDiscovery();

    auto allCreated = std::promise<void>();
    auto allRemoved = std::promise<void>();

    const auto discoveryLookupKey = SilKit::Core::Discovery::controllerTypeDataPublisher + "/"
                                    + SilKit::Core::Discovery::supplKeyDataPublisherTopic + "/" + topic + "/"
                                    + SilKit::Core::Discovery::supplKeyDataPublisherPubLabels + "/";

    std::string controllerType = SilKit::Core::Discovery::controllerTypeDataPublisher;

    std::string mediaType = "";
    std::string topicCopy = topic;
    std::vector<SilKit::Services::MatchingLabel> labels;

    // Participants are already there, so the registration will trigger the provided handler immediately
    subscriberServiceDiscovery->RegisterSpecificServiceDiscoveryHandler(
        [numberOfServices, &allRemoved, &allCreated, &createdServiceNames, &removedServiceNames, publisherName](
            auto discoveryType, const auto& service) {
        switch (discoveryType)
        {
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::Invalid:
            break;
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated:
            if (service.GetParticipantName() == publisherName)
            {
                createdServiceNames.push_back(service.GetServiceName());
                if (createdServiceNames.size() == numberOfServices)
                {
                    allCreated.set_value();
                }
            }
            break;
        case SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved:
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
    },
        controllerType, topicCopy, labels);

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
