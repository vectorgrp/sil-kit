// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include <numeric>
#include <algorithm>

// DEBUG
#include <fstream>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"

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

void SystemStateHandler(sync::ISystemController* controller, sync::SystemState newState, const Config& ibConfig)
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

auto BuildConfig(uint32_t participantCount, Middleware middleware, bool useNetworkSimulator) -> ib::cfg::Config
{
	ConfigBuilder benchmarkConfig("BenchmarkConfigGenerated");
	auto&& simulationSetup = benchmarkConfig.SimulationSetup();
    std::vector<std::string> networkSimulatorNames;

	for (unsigned int participantCounter = 0; participantCounter < participantCount; participantCounter++)
	{
		std::stringstream participantNameBuilder;
		participantNameBuilder << "Participant";
		participantNameBuilder << participantCounter;

		auto &&participantBuilder = simulationSetup.AddParticipant(participantNameBuilder.str());
		participantBuilder.WithParticipantId(participantCounter);
        participantBuilder.AddParticipantController().WithSyncType(ib::cfg::SyncType::DistributedTimeQuantum);

        participantBuilder.ConfigureLogger()
            .WithFlushLevel(ib::mw::logging::Level::Trace)
            .AddSink(ib::cfg::Sink::Type::Stdout)
                .WithLogLevel(ib::mw::logging::Level::Trace);

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
			linkNameBuilder << "AndParticipant";
			linkNameBuilder << otherParticipantsCounter;

            if (useNetworkSimulator)
            {
                std::stringstream networkSimulatorNameBuilder;
                networkSimulatorNameBuilder << "NetworkSimulatorFor";
                networkSimulatorNameBuilder << linkNameBuilder.str();

                networkSimulatorNames.push_back(networkSimulatorNameBuilder.str());

                participantBuilder.AddNetworkSimulator(networkSimulatorNameBuilder.str()).WithLinks({ linkNameBuilder.str() });
            }

			participantBuilder->AddGenericPublisher(publisherNameBuilder.str()).WithLink(linkNameBuilder.str());
			participantBuilder->AddGenericSubscriber(subscriberNameBuilder.str()).WithLink(linkNameBuilder.str());
		}
	}

    if (useNetworkSimulator)
    {
        for (auto&& name : networkSimulatorNames)
        {
            auto&& participant = simulationSetup.AddParticipant(name);
            participant.AddParticipantController().WithSyncType(ib::cfg::SyncType::DistributedTimeQuantum).Parent()->AddNetworkSimulator(name);
            participant.ConfigureLogger()
                .WithFlushLevel(ib::mw::logging::Level::Trace)
                .AddSink(ib::cfg::Sink::Type::Stdout)
                .WithLogLevel(ib::mw::logging::Level::Trace);
        }
    }

	simulationSetup.AddParticipant("Master")
		.AsSyncMaster()
            .ConfigureLogger()
                .WithFlushLevel(ib::mw::logging::Level::Trace)
                .AddSink(ib::cfg::Sink::Type::Stdout)
                    .WithLogLevel(ib::mw::logging::Level::Trace);

	simulationSetup.ConfigureTimeSync()
		.WithLooseSyncPolicy()
		.WithTickPeriod(1000000ns);

    benchmarkConfig.WithActiveMiddleware(middleware);

	auto config = benchmarkConfig.Build();

    // DEBUG
    std::ofstream fileOut;
    fileOut.open("C:\\Users\\vikatik\\Desktop\\temp_config.json");
    fileOut << config.ToJsonString() << std::endl;
    fileOut.close();

	return config;
}

void ParticipantsThread(
	uint32_t payloadSizeInBytes,
	std::chrono::seconds simulationDuration,
	const Config& ibConfig,
	const Participant& thisParticipant,
	uint32_t domainId,
	bool isVerbose)
{
	auto comAdapter = CreateComAdapterImpl(ibConfig, thisParticipant.name);
	comAdapter->joinIbDomain(domainId);
	auto participantController = comAdapter->GetParticipantController();

	std::vector<ib::sim::generic::IGenericPublisher*> publishers;
	std::vector<ib::sim::generic::IGenericSubscriber*> subscribers;

	for (auto& genericPublisher : thisParticipant.genericPublishers)
	{
		publishers.push_back(comAdapter->CreateGenericPublisher(genericPublisher.name));
	}
	for (auto& genericSubscriber : thisParticipant.genericSubscribers)
	{
		auto thisSubscriber = comAdapter->CreateGenericSubscriber(genericSubscriber.name);
		thisSubscriber->SetReceiveMessageHandler(ReceiveMessage);
		subscribers.push_back(thisSubscriber);
	}

	participantController->SetSimulationTask(
		[payloadSizeInBytes, simulationDuration, participantController, &publishers, isVerbose](std::chrono::nanoseconds now) {

		if (now > simulationDuration)
		{
			participantController->Stop("Simulation done");
		}

		if (isVerbose)
		{
			auto simulationDurationInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(simulationDuration);
			auto durationOfOneSimulationPercentile = simulationDurationInNs / 100;

			if (now % durationOfOneSimulationPercentile < 1000000ns)
			{
				std::cout << ".";
			}
		}

		for (auto &publisher : publishers)
		{
			PublishMessage(publisher, payloadSizeInBytes);
		}
	});

	participantController->Run();
	return;
}

/**************************************************************************************************
* Main Function
**************************************************************************************************/
int main(int argc, char** argv)
{
	try
	{
		std::vector<std::chrono::nanoseconds> measuredRealDurations;

		uint32_t numberOfParticipants = 4;
		std::chrono::seconds simulationDuration = 1s;
		uint32_t simulationRepeats = 5;
		uint32_t payloadSizeInBytes = 100;
		uint32_t domainId = 42;
        Middleware usedMiddleware = Middleware::VAsio;
        bool useNetworkSimulator = true;

		if (argc >= 8)
		{
			std::cout << "Too many arguments." << std::endl;
			std::cout << "Start benchmark with " << argv[0];
			std::cout << " [numberOfParticipants] [simulationDuration] [simulationRepeats] [payloadSizeInBytes] [domainId] [useFastRTPS]" << std::endl;
			return -1;
		}

		switch (argc)
		{
        case 7: if (std::stoul(argv[6])) { usedMiddleware = Middleware::FastRTPS; }
            // [[fallthrough]]
		case 6: domainId = domainId = static_cast<uint32_t>(std::stoul(argv[5]));
			// [[fallthrough]]
		case 5: payloadSizeInBytes = static_cast<uint32_t>(std::stoul(argv[4]));
			// [[fallthrough]]
		case 4: simulationRepeats = static_cast<uint32_t>(std::stoul(argv[3]));
			// [[fallthrough]]
		case 3: simulationDuration = std::chrono::seconds(static_cast<uint32_t>(std::stoul(argv[2])));
			// [[fallthrough]]
		case 2: numberOfParticipants = static_cast<uint32_t>(std::stoul(argv[1]));
			break;
		default:
			std::cout << "Using default benchmark parameters (4 participants, 1 second, 5 repeats, 100 bytes, domainId 42, 0);" << std::endl;
		}

		if (numberOfParticipants < 2 || simulationDuration < 1s || simulationRepeats < 1 || payloadSizeInBytes < 1)
		{
			std::cout << "Invalid argument." << std::endl;
			std::cout << "Start benchmark with " << argv[0];
			std::cout << " [numberOfParticipants] [simulationDuration] [simulationRepeats] [payloadSizeInBytes] [domainId] [useFastRTPS]" << std::endl;
			return -1;
		}

		for (uint32_t simulationRun = 1; simulationRun <= simulationRepeats; simulationRun++)
		{
            auto ibConfig = BuildConfig(numberOfParticipants, usedMiddleware, useNetworkSimulator);
            std::vector<std::thread> threads;
            std::thread VAsioRegistryThread;
            
            if (usedMiddleware == Middleware::VAsio)
            {
                VAsioRegistryThread = std::thread([ibConfig, domainId] {
                    VAsioRegistry registry{ ibConfig };
                    auto future = registry.ProvideDomain(domainId);
                    future.wait();
                });
            }

            if (useNetworkSimulator == true)
            {
                std::cout << "Please start all participating network simulators now." << std::endl;
                std::cout << "Press enter to continue." << std::endl;
                std::cin.ignore();
            }

			auto startTimestamp = std::chrono::system_clock::now();

			for (auto &&thisParticipant : ibConfig.simulationSetup.participants)
			{
                if (thisParticipant.name == "Master")
                {
                    continue;
                }
                if (thisParticipant.name.rfind("NetworkSimulator", 0) == 0)
                {
                    continue;
                }

				bool isVerbose = false;
                if (thisParticipant.id == 0)
                {
                    isVerbose = true;
                }
				threads.push_back(std::thread([payloadSizeInBytes, simulationDuration, ibConfig, thisParticipant, domainId, isVerbose] {
					ParticipantsThread(payloadSizeInBytes,
						simulationDuration,
						ibConfig,
						thisParticipant,
						domainId,
						isVerbose);
				}));
			}

			auto comAdapter = CreateComAdapterImpl(ibConfig, "Master");
			comAdapter->joinIbDomain(domainId);

			auto controller = comAdapter->GetSystemController();
			auto monitor = comAdapter->GetSystemMonitor();

			monitor->RegisterSystemStateHandler([controller, &ibConfig](sync::SystemState newState) {
				SystemStateHandler(controller, newState, ibConfig);
			});

			monitor->RegisterParticipantStatusHandler([controller](const sync::ParticipantStatus& newStatus) {
				ParticipantStatusHandler(controller, newStatus);
			});

			for (auto&& thread : threads)
			{
				thread.join();
			}

            if (usedMiddleware == Middleware::VAsio)
            {
                comAdapter.reset();
                VAsioRegistryThread.join();
            }

			auto endTimestamp = std::chrono::system_clock::now();
			auto duration = endTimestamp - startTimestamp;

			measuredRealDurations.push_back(duration);
		}

		auto minDuration = *std::max_element(measuredRealDurations.begin(), measuredRealDurations.end());
		auto maxDuration = *std::min_element(measuredRealDurations.begin(), measuredRealDurations.end());
		auto averageDuration = std::accumulate(measuredRealDurations.begin(), measuredRealDurations.end(), 0ns) / measuredRealDurations.size();

		std::cout << "\nSimulation duration: " << simulationDuration;
		std::cout << " with " << numberOfParticipants << " partcipants, " << simulationRepeats << " repeats, ";
		std::cout << payloadSizeInBytes << " bytes / message\n\n";

		if (simulationRepeats > 1)
		{
			uint32_t runNumber = 1;
			for (auto&& duration : measuredRealDurations)
			{
				std::cout << "Simulation run " << runNumber << ": " << duration << "\n";

				runNumber++;
			}
			std::cout << std::endl;
		}

		std::cout << "Average realtime duration: " << averageDuration;

		if (simulationRepeats > 1)
		{
			std::cout << " (min " << minDuration << ", max " << maxDuration << ")";
		}
		std::cout << std::endl;
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

