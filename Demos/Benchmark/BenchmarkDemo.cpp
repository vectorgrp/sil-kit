// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include <numeric>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include "ib/cfg/Config.hpp"

using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
	auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
	out << seconds.count() << "s";
	return out;
}

void PublishMessage(IGenericPublisher* publisher, unsigned int payloadSizeInBytes)
{
	std::vector<uint8_t> data(payloadSizeInBytes, '*');
	publisher->Publish(std::move(data));
}

void ReceiveMessage(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
{
	// do nothing
}

void ParticipantStatusHandler(sync::ISystemController* controller, const sync::ParticipantStatus& newStatus)
{
	switch (newStatus.state)
	{
	case sync::ParticipantState::Stopped:
		controller->Stop();
		break;
	}
}

void SystemStateHandler(sync::ISystemController* controller, sync::SystemState newState, const ib::cfg::Config& ibConfig)
{
	switch (newState)
	{
	case ib::mw::sync::SystemState::Idle:
	{
		for (auto&& participant : ibConfig.simulationSetup.participants)
		{
			if (participant.name == "master")
				continue;
			controller->Initialize(participant.id);
		}
		break;
	}

	case ib::mw::sync::SystemState::Initialized:
		controller->Run();
		break;

	case ib::mw::sync::SystemState::Stopped:
		controller->Shutdown();
		break;

	default:
		std::cout << "New SystemState " << to_string(newState) << std::endl;
	}
}

auto buildConfig(unsigned int participantCount) -> ib::cfg::Config
{
	ConfigBuilder benchmarkConfig("BenchmarkConfigGenerated");
	auto&& simulationSetup = benchmarkConfig.SimulationSetup();

	for (unsigned int participantCounter = 0; participantCounter < participantCount; participantCounter++)
	{
		std::stringstream participantNameBuilder;
		participantNameBuilder << "Participant";
		participantNameBuilder << participantCounter;

		auto &&participantBuilder = simulationSetup.AddParticipant(participantNameBuilder.str());
		participantBuilder.WithParticipantId(participantCounter);
		participantBuilder.WithSyncType(ib::cfg::SyncType::DiscreteTime);

		for (unsigned int otherParticipantsCounter = 0; otherParticipantsCounter < participantCount; otherParticipantsCounter++)
		{
			if (participantCounter == otherParticipantsCounter)
				continue;

			std::stringstream publisherNameBuilder;
			publisherNameBuilder << "PublisherOfParticipant";
			publisherNameBuilder << participantCounter;
			publisherNameBuilder << "ToParticipant";
			publisherNameBuilder << otherParticipantsCounter;

			std::stringstream subscriberNameBuilder;
			subscriberNameBuilder << "SubscriberOfParticipant";
			subscriberNameBuilder << participantCounter;
			subscriberNameBuilder << "FromParticipant";
			subscriberNameBuilder << otherParticipantsCounter;

			std::stringstream linkNameBuilder;
			linkNameBuilder << "LinkBetweenParticipant";
			linkNameBuilder << participantCounter;
			linkNameBuilder << "ToParticipant";
			linkNameBuilder << otherParticipantsCounter;

			participantBuilder->AddGenericPublisher(publisherNameBuilder.str()).WithLink(linkNameBuilder.str());
			participantBuilder->AddGenericSubscriber(subscriberNameBuilder.str()).WithLink(linkNameBuilder.str());
		}
	}

	simulationSetup.AddParticipant("Master")
		.AsSyncMaster();

	simulationSetup.ConfigureTimeSync()
		.WithLooseSyncPolicy()
		.WithTickPeriod(1000000ns);

	auto config = benchmarkConfig.Build();

	return config;
}

/**************************************************************************************************
* Main Function
**************************************************************************************************/

int main(int argc, char** argv)
{
	try
	{
		std::vector<std::chrono::nanoseconds> measuredRealDurations;

		std::chrono::seconds simulationDuration = 1s;
		uint32_t simulationRepeats = 5;
		uint32_t payloadSizeInBytes = 100;
		uint32_t domainId = 42;

		if (argc > 5)
		{
			std::cout << "Too many arguments." << std::endl;
			std::cout << "Start benchmark with " << argv[0] << " [simulationDuration] [simulationRepeats] [payloadSizeInBytes] [domainId]" << std::endl;
			return -1;
		}

		switch (argc)
		{
		case 5: domainId = domainId = static_cast<uint32_t>(std::stoul(argv[4]));
			// [[fallthrough]]
		case 4: payloadSizeInBytes = static_cast<uint32_t>(std::stoul(argv[3]));
			// [[fallthrough]]
		case 3: simulationRepeats = static_cast<uint32_t>(std::stoul(argv[2]));
			// [[fallthrough]]
		case 2: simulationDuration = std::chrono::seconds(static_cast<uint32_t>(std::stoul(argv[1])));
			// [[fallthrough]]
		default:
			std::cout << "Using default benchmark parameters (1 second, 5 repeats, 100 bytes, domainId 42);" << std::endl;
		}

		for (uint32_t simulationRun = 1; simulationRun <= simulationRepeats; simulationRun++)
		{
			auto startTimestamp = std::chrono::system_clock::now();
			auto ibConfig = buildConfig(4);
			auto simulationTime = 1s;
			std::vector<std::thread> threads;

			for (auto &&thisParticipant : ibConfig.simulationSetup.participants)
			{
				if (thisParticipant.name == "Master")
					continue;

				threads.push_back(std::thread([payloadSizeInBytes, simulationDuration, &ibConfig, &thisParticipant, &domainId] {

					auto comAdapter = ib::CreateComAdapter(ibConfig, thisParticipant.name, domainId);
					auto participantController = comAdapter->GetParticipantController();
					participantController->SetPeriod(1ms);

					std::vector<ib::sim::generic::IGenericPublisher*> publishers;
					std::vector<ib::sim::generic::IGenericSubscriber*> subscribers;

					for (auto &genericPublisher : thisParticipant.genericPublishers)
					{
						publishers.push_back(comAdapter->CreateGenericPublisher(genericPublisher.name));
					}
					for (auto &genericSubscriber : thisParticipant.genericSubscribers)
					{
						auto subscriber = comAdapter->CreateGenericSubscriber(genericSubscriber.name);
						subscriber->SetReceiveMessageHandler(ReceiveMessage);
						subscribers.push_back(subscriber);
					}

					participantController->SetSimulationTask([payloadSizeInBytes, simulationDuration, participantController, &publishers](std::chrono::nanoseconds now) {
						for (auto &publisher : publishers)
						{
							PublishMessage(publisher, payloadSizeInBytes);

							if (now > simulationDuration)
							{
								participantController->Stop("Simulation done");
							}
						}
					});

					participantController->Run();
				}));
			}

			auto comAdapter = ib::CreateComAdapter(ibConfig, "Master", domainId);

			auto controller = comAdapter->GetSystemController();
			auto monitor = comAdapter->GetSystemMonitor();

			monitor->RegisterSystemStateHandler([controller, &ibConfig](sync::SystemState newState) {
				SystemStateHandler(controller, newState, ibConfig);
			});

			monitor->RegisterParticipantStatusHandler([controller](const sync::ParticipantStatus& newStatus) {
				ParticipantStatusHandler(controller, newStatus);
			});

			for (auto &thread : threads)
			{
				thread.join();
			}

			auto endTimestamp = std::chrono::system_clock::now();
			auto duration = endTimestamp - startTimestamp;

			measuredRealDurations.push_back(duration);
		}

		auto averageDuration = 0ns;
		for (auto&& duration : measuredRealDurations)
		{
			averageDuration += duration;
		}
		averageDuration /= measuredRealDurations.size();

		std::cout << "\n========================================" << std::endl;
		std::cout << "Simulation duration: " << simulationDuration;
		std::cout << " with " << simulationRepeats << " repeats (";
		std::cout << payloadSizeInBytes << " bytes / message)" << std::endl;
		std::cout << "Average realtime duration: " << averageDuration << std::endl << std::endl;

		uint32_t runNumber = 1;
		for (auto&& duration : measuredRealDurations)
		{
			std::cout << "Simulation run " << runNumber << ": ";
			std::cout << "Realtime duration: " << duration << std::endl;

			runNumber++;
		}
	}
	catch (const ib::cfg::Misconfiguration& error)
	{
		std::cerr << "Invalid configuration: " << error.what() << std::endl;
		std::cout << "Press enter to stop the process..." << std::endl;
		std::cin.ignore();
		return -2;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Something went wrong: " << error.what() << std::endl;
		std::cout << "Press enter to stop the process..." << std::endl;
		std::cin.ignore();
		return -3;
	}

	return 0;
}

