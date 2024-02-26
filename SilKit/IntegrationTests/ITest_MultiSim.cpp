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

#include "silkit/SilKit.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "gtest/gtest.h"


namespace {


struct Simulation
{
    struct SystemController
    {
        std::unique_ptr<SilKit::IParticipant> participant;
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController{nullptr};
        SilKit::Services::Orchestration::ILifecycleService* lifecycleService{nullptr};

        std::promise<void> comReadyPromise;
        std::future<void> comReadyFuture;

        std::promise<void> startingPromise;
        std::future<void> startingFuture;

        std::future<SilKit::Services::Orchestration::ParticipantState> doneFuture;
    };

    struct Publisher
    {
        std::string participantName;

        std::unique_ptr<SilKit::IParticipant> participant;
        SilKit::Services::Orchestration::ILifecycleService* lifecycleService{nullptr};
        SilKit::Services::PubSub::IDataPublisher* publisher{nullptr};

        std::vector<uint8_t> message;

        std::promise<void> comReadyPromise;
        std::future<void> comReadyFuture;

        std::promise<void> startingPromise;
        std::future<void> startingFuture;

        std::future<SilKit::Services::Orchestration::ParticipantState> doneFuture;
    };

    struct Subscriber
    {
        std::string participantName;

        std::unique_ptr<SilKit::IParticipant> participant;
        SilKit::Services::Orchestration::ILifecycleService* lifecycleService{nullptr};
        SilKit::Services::PubSub::IDataSubscriber* subscriber{nullptr};

        std::vector<uint8_t> message;

        std::promise<void> comReadyPromise;
        std::future<void> comReadyFuture;

        std::promise<void> startingPromise;
        std::future<void> startingFuture;

        std::promise<void> receivedDataPromise;
        std::future<void> receivedDataFuture;

        std::future<SilKit::Services::Orchestration::ParticipantState> doneFuture;
    };

    void CreateSystemController(std::vector<std::string> requiredParticipantNames = {})
    {
        systemController.participant =
            SilKit::CreateParticipant(participantConfiguration, "SystemController", registryUri);

        // collect participant names

        requiredParticipantNames.emplace_back("SystemController");

        for (const auto& pair : publishers)
        {
            const auto& participantName = pair.first;
            requiredParticipantNames.emplace_back(participantName);
        }

        for (const auto& pair : subscribers)
        {
            const auto& participantName = pair.first;
            requiredParticipantNames.emplace_back(participantName);
        }

        // create and setup system controller

        systemController.systemController =
            SilKit::Experimental::Participant::CreateSystemController(systemController.participant.get());

        systemController.systemController->SetWorkflowConfiguration(
            SilKit::Services::Orchestration::WorkflowConfiguration{std::move(requiredParticipantNames)});

        // create lifecycle service

        systemController.lifecycleService = systemController.participant->CreateLifecycleService(
            SilKit::Services::Orchestration::LifecycleConfiguration{
                SilKit::Services::Orchestration::OperationMode::Coordinated});

        systemController.lifecycleService->SetCommunicationReadyHandler([systemController = &systemController] {
            systemController->comReadyPromise.set_value();
        });

        systemController.lifecycleService->SetStartingHandler([systemController = &systemController] {
            systemController->startingPromise.set_value();
        });

        systemController.comReadyFuture = systemController.comReadyPromise.get_future();
        systemController.startingFuture = systemController.startingPromise.get_future();
    }

    auto CreatePublisher(const std::string& participantName, const std::string& topic) -> Publisher&
    {
        auto publisher = std::make_unique<Publisher>();
        publisher->participantName = participantName;

        publisher->participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        // create lifecycle service

        publisher->lifecycleService =
            publisher->participant->CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration{
                SilKit::Services::Orchestration::OperationMode::Coordinated});

        publisher->lifecycleService->SetCommunicationReadyHandler([publisher = publisher.get()] {
            publisher->comReadyPromise.set_value();
        });

        publisher->lifecycleService->SetStartingHandler([publisher = publisher.get()] {
            publisher->startingPromise.set_value();
        });

        publisher->comReadyFuture = publisher->comReadyPromise.get_future();
        publisher->startingFuture = publisher->startingPromise.get_future();

        // create publisher

        SilKit::Services::PubSub::PubSubSpec spec{topic, ""};
        publisher->publisher = publisher->participant->CreateDataPublisher("Publisher", spec);

        // store the publisher object

        auto& publisherRef = *publisher;
        publishers.emplace(participantName, std::move(publisher));

        return publisherRef;
    }

    auto CreateSubscriber(const std::string& participantName, const std::string& topic) -> Subscriber&
    {
        auto subscriber = std::make_unique<Subscriber>();
        subscriber->participantName = participantName;

        subscriber->participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        // create lifecycle service

        subscriber->lifecycleService =
            subscriber->participant->CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration{
                SilKit::Services::Orchestration::OperationMode::Coordinated});

        subscriber->lifecycleService->SetCommunicationReadyHandler([subscriber = subscriber.get()] {
            subscriber->comReadyPromise.set_value();
        });

        subscriber->lifecycleService->SetStartingHandler([subscriber = subscriber.get()] {
            subscriber->startingPromise.set_value();
        });

        subscriber->comReadyFuture = subscriber->comReadyPromise.get_future();
        subscriber->startingFuture = subscriber->startingPromise.get_future();

        // create subscriber

        SilKit::Services::PubSub::PubSubSpec spec{topic, ""};
        subscriber->subscriber = subscriber->participant->CreateDataSubscriber(
            "Subscriber", spec,
            [subscriber = subscriber.get()](SilKit::Services::PubSub::IDataSubscriber*,
                                            const SilKit::Services::PubSub::DataMessageEvent& event) {
                if (event.data.size() != subscriber->message.size())
                {
                    subscriber->receivedDataPromise.set_exception(
                        std::make_exception_ptr(std::runtime_error{"invalid message size"}));
                }

                if (!ItemsAreEqual(event.data, SilKit::Util::ToSpan(subscriber->message)))
                {
                    subscriber->receivedDataPromise.set_exception(
                        std::make_exception_ptr(std::runtime_error{"invalid message data"}));
                }

                subscriber->receivedDataPromise.set_value();
            });

        subscriber->receivedDataFuture = subscriber->receivedDataPromise.get_future();

        // store the subscriber object

        auto& subscriberRef = *subscriber;
        subscribers.emplace(participantName, std::move(subscriber));

        return subscriberRef;
    }

    void StartAll()
    {
        systemController.doneFuture = systemController.lifecycleService->StartLifecycle();

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            publisher->doneFuture = publisher->lifecycleService->StartLifecycle();
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            subscriber->doneFuture = subscriber->lifecycleService->StartLifecycle();
        }
    }

    void WaitAllCommunicationReady(std::chrono::nanoseconds timeout)
    {
        ASSERT_EQ(systemController.comReadyFuture.wait_for(timeout), std::future_status::ready) << registryUri;

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            ASSERT_EQ(publisher->comReadyFuture.wait_for(timeout), std::future_status::ready)
                << registryUri << " " << publisher->participantName;
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            ASSERT_EQ(subscriber->comReadyFuture.wait_for(timeout), std::future_status::ready)
                << registryUri << " " << subscriber->participantName;
        }

        systemController.comReadyFuture.get();

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            publisher->comReadyFuture.get();
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            subscriber->comReadyFuture.get();
        }
    }

    void WaitAllStarting(std::chrono::nanoseconds timeout)
    {
        ASSERT_EQ(systemController.startingFuture.wait_for(timeout), std::future_status::ready) << registryUri;

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            ASSERT_EQ(publisher->startingFuture.wait_for(timeout), std::future_status::ready)
                            << registryUri << " " << publisher->participantName;
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            ASSERT_EQ(subscriber->startingFuture.wait_for(timeout), std::future_status::ready)
                            << registryUri << " " << subscriber->participantName;
        }

        systemController.startingFuture.get();

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            publisher->startingFuture.get();
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            subscriber->startingFuture.get();
        }
    }

    void PublishAll()
    {
        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            publisher->publisher->Publish(SilKit::Util::ToSpan(publisher->message));
        }
    }

    void WaitAllReceived(std::chrono::nanoseconds timeout)
    {
        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            ASSERT_EQ(subscriber->receivedDataFuture.wait_for(timeout), std::future_status::ready)
                << registryUri << " " << subscriber->participantName;
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            subscriber->receivedDataFuture.get();
        }
    }

    void WaitAllDone(std::chrono::nanoseconds timeout)
    {
        ASSERT_EQ(systemController.doneFuture.wait_for(timeout), std::future_status::ready) << registryUri;

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            ASSERT_EQ(publisher->doneFuture.wait_for(timeout), std::future_status::ready)
                << registryUri << " " << publisher->participantName;
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            ASSERT_EQ(subscriber->doneFuture.wait_for(timeout), std::future_status::ready)
                << registryUri << " " << subscriber->participantName;
        }

        systemController.doneFuture.get();

        for (const auto& pair : publishers)
        {
            const auto& publisher = pair.second;
            publisher->doneFuture.get();
        }

        for (const auto& pair : subscribers)
        {
            const auto& subscriber = pair.second;
            subscriber->doneFuture.get();
        }
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
    std::string registryUri;

    SystemController systemController;
    std::unordered_map<std::string, std::unique_ptr<Publisher>> publishers;
    std::unordered_map<std::string, std::unique_ptr<Subscriber>> subscribers;
};


struct ITest_MultiSim : testing::Test
{
    ITest_MultiSim()
    {
        registryConfiguration = SilKit::Config::ParticipantConfigurationFromString(R"()");
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
        registryUri = registry->StartListening("silkit://127.0.0.1:0");

        participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(R"()");
    }

    auto MakeSimulation(const std::string& simulationName = "") -> std::unique_ptr<Simulation>
    {
        auto simulation = std::make_unique<Simulation>();

        simulation->participantConfiguration = participantConfiguration;

        if (simulationName.empty())
        {
            simulation->registryUri = registryUri;
        }
        else
        {
            simulation->registryUri = registryUri + '/' + simulationName;
        }

        return simulation;
    }

    void MakePublisherSubscriberPair(Simulation& simulation, const std::string& publisherParticipantName,
                                     const std::string& subscriberParticipantName, const std::string& topic,
                                     const std::vector<uint8_t>& message)
    {
        auto& publisher = simulation.CreatePublisher(publisherParticipantName, topic);
        publisher.message = message;

        auto& subscriber = simulation.CreateSubscriber(subscriberParticipantName, topic);
        subscriber.message = message;
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> registryConfiguration;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;
    std::string registryUri;

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
};


TEST_F(ITest_MultiSim, simultaneous_simulations_with_same_participant_names_run)
{
    const std::chrono::seconds TIMEOUT{5};

    std::vector<std::unique_ptr<Simulation>> simulations;

    for (int i = 0; i < 5; ++i)
    {
        std::string simulationName;

        if (i != 0)
        {
            simulationName = "simulation/" + std::to_string(i);
        }

        auto simulation = MakeSimulation(simulationName);

        for (int j = 0; j < i + 1; ++j)
        {
            auto publisherParticipantName = std::string{"P "} + std::to_string(j);
            auto subscriberParticipantName = std::string{"S "} + std::to_string(j);
            auto topicName = std::string{"Topic "} + std::to_string(j);

            std::vector<uint8_t> message;
            message.assign(static_cast<decltype(message)::size_type>(j), 'X');

            MakePublisherSubscriberPair(*simulation, publisherParticipantName, subscriberParticipantName, topicName,
                                        message);
        }

        simulations.emplace_back(std::move(simulation));
    }

    for (const auto& simulation : simulations)
    {
        simulation->CreateSystemController();
    }

    for (const auto& simulation : simulations)
    {
        simulation->StartAll();
    }

    for (const auto& simulation : simulations)
    {
        simulation->WaitAllCommunicationReady(TIMEOUT);
    }

    for (const auto& simulation : simulations)
    {
        simulation->WaitAllStarting(TIMEOUT);
    }

    for (const auto& simulation : simulations)
    {
        simulation->PublishAll();
    }

    for (const auto& simulation : simulations)
    {
        simulation->WaitAllReceived(TIMEOUT);
    }

    for (const auto& simulation : simulations)
    {
        simulation->systemController.lifecycleService->Stop("done");
    }

    for (const auto& simulation : simulations)
    {
        simulation->WaitAllDone(TIMEOUT);
    }
}


TEST_F(ITest_MultiSim, same_participant_name_different_simulation_names_work)
{
    {
        auto a = MakeSimulation("");
        a->CreatePublisher("P", "T");

        auto b = MakeSimulation("X");
        b->CreatePublisher("P", "T");
    }

    {
        auto a = MakeSimulation("X");
        a->CreatePublisher("P", "T");

        auto b = MakeSimulation("Y");
        b->CreatePublisher("P", "T");
    }
}


TEST_F(ITest_MultiSim, same_participant_name_single_simulation_fails)
{
    {
        auto a = MakeSimulation("");
        a->CreatePublisher("P", "T");

        auto b = MakeSimulation("");
        ASSERT_THROW(b->CreatePublisher("P", "T"), SilKit::SilKitError);
    }

    {
        auto a = MakeSimulation("X");
        a->CreatePublisher("P", "T");

        auto b = MakeSimulation("X");
        ASSERT_THROW(b->CreatePublisher("P", "T"), SilKit::SilKitError);
    }
}


} //end namespace
