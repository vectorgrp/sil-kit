// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/core/sync/all.hpp"
#include "silkit/core/sync/string_utils.hpp"

using namespace SilKit::Core;
using namespace SilKit::Services::Rpc;
using namespace std::chrono_literals;

std::chrono::milliseconds callReturnTimeout{ 5000ms };

std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& v)
{
    os << "[ ";
    for (auto i : v)
        os << static_cast<int>(i) << " ";
    os << "]";
    return os;
}

void Call(IRpcClient* client)
{
    std::vector<uint8_t> argumentData{
        static_cast<uint8_t>(rand() % 10),
        static_cast<uint8_t>(rand() % 10),
        static_cast<uint8_t>(rand() % 10) };

    auto callHandle = client->Call(argumentData);
    if (callHandle)
    {
        std::cout << "<< Calling with argumentData=" << argumentData << std::endl;
    }
}

void CallReturn(IRpcClient* /*cbClient*/, RpcCallResultEvent event)
{
    switch (event.callStatus)
    {
    case RpcCallStatus::Success:
        std::cout << ">> Call returned with resultData=" << event.resultData << std::endl;
        break;
    case RpcCallStatus::ServerNotReachable:
        std::cout << "Warning: Call failed with RpcCallStatus::ServerNotReachable" << std::endl;
        break;
    case RpcCallStatus::UndefinedError:
        std::cout << "Warning: Call failed with RpcCallStatus::UndefinedError" << std::endl;
        break;
    }
}

void RemoteFunc_Add100(IRpcServer* server, RpcCallEvent event)
{
    auto returnData{event.argumentData};
    for (auto& v : returnData)
    {
        v += 100;
    }

    std::cout << ">> Received call with argumentData=" << event.argumentData
              << ", returning resultData=" << returnData << std::endl;

    server->SubmitResult(event.callHandle, returnData);
}

void RemoteFunc_Sort(IRpcServer* server, RpcCallEvent event)
{
    auto returnData{event.argumentData};
    std::sort(returnData.begin(), returnData.end());
    std::cout << ">> Received call with argumentData=" << event.argumentData
              << ", returning resultData=" << returnData << std::endl;

    server->SubmitResult(event.callHandle, returnData);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [registryUri]" << std::endl
                  << "Use \"Server\" or \"Client\" as <ParticipantName>." << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        auto registryUri = "silkit://localhost:8500";
        if (argc >= 4)
        {
            registryUri = argv[3];
        }

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        auto* lifecycleService = participant->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        lifecycleService->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });
        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        timeSyncService->SetPeriod(1s);
        if (participantName == "Client")
        {
            std::string clientAFunctionName = "Add100";
            auto mediaTypeClientA = std::string{"application/octet-stream"};
            std::map<std::string, std::string> labelsClientA{{"KeyA", "ValA"}};
            auto clientA = participant->CreateRpcClient("ClientCtrl1", clientAFunctionName, mediaTypeClientA,
                                                        labelsClientA, &CallReturn);

            std::string clientBFunctionName = "Sort";
            auto mediaTypeClientB = std::string{""};
            std::map<std::string, std::string> labelsClientB{{"KeyC", "ValC"}};
            auto clientB =
                participant->CreateRpcClient("ClientCtrl2", "Sort", mediaTypeClientB, labelsClientB, &CallReturn);

            RpcDiscoveryResultHandler discoveryResultsHandler =
                [](const std::vector<RpcDiscoveryResult>& discoveryResults) {
                    std::cout << ">> Found remote RpcServers:" << std::endl;
                    for (const auto& entry : discoveryResults)
                    {
                        std::cout << "   " << entry << std::endl;
                    }
                };

            timeSyncService->SetSimulationTask(
                [clientA, clientB, &participant, &discoveryResultsHandler](std::chrono::nanoseconds now,
                                                                           std::chrono::nanoseconds /*duration*/) {
                    if (now == 0ms) 
                    {
                        participant->DiscoverRpcServers("", "", {}, discoveryResultsHandler);
                    }

                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    Call(clientA);
                    Call(clientB);
                });
        }
        else // "Server"
        {
            auto mediaTypeServerA = std::string{"application/octet-stream"};
            std::map<std::string, std::string> labelsServerA{{"KeyA", "ValA"}, {"KeyB", "ValB"}};
            participant->CreateRpcServer("ServerCtrl1", "Add100", mediaTypeServerA, labelsServerA, &RemoteFunc_Add100);

            auto mediaTypeServerB = std::string{"application/json"};
            std::map<std::string, std::string> labelsServerB{{"KeyC", "ValC"}, {"KeyD", "ValD"}};
            participant->CreateRpcServer("ServerCtrl2", "Sort", mediaTypeServerB, labelsServerB, &RemoteFunc_Sort);

            timeSyncService->SetSimulationTask(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(3s);
            });
        }

        auto lifecycleFuture = lifecycleService->StartLifecycleWithSyncTime(timeSyncService, {true, true});
        auto finalState = lifecycleFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const SilKit::ConfigurationError& error)
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
