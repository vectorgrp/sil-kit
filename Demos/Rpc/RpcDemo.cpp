// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <stdlib.h>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim::rpc;
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
        std::cout << "<< Calling '" << client->Config().name << "' with argumentData=" << argumentData << std::endl;
    }
}

void CallReturn(IRpcClient* client, IRpcCallHandle* callHandle,
    const CallStatus callStatus, const std::vector<uint8_t>& returnData)
{
    switch (callStatus)
    {
    case CallStatus::Success:
        std::cout << ">> '" << client->Config().name << "' returned with returnData=" << returnData << std::endl;
        break;
    case CallStatus::ServerNotReachable:
        std::cout << "Warning: Calling '" << client->Config().name << "' failed with CallStatus::ServerNotReachable" << std::endl;
        break;
    case CallStatus::UndefinedError:
        std::cout << "Warning: Calling '" << client->Config().name << "' failed with CallStatus::UndefinedError" << std::endl;
        break;
    }
}

void RemoteFunc_Add100(IRpcServer* server, IRpcCallHandle* callHandle, const std::vector<uint8_t>& argumentData)
{
    auto returnData{ argumentData };
    for (auto& v : returnData)
        v += 100;
    std::cout << ">> Received call to '" << server->Config().name << "' with argumentData=" << argumentData
              << ", returning resultData=" << returnData << std::endl;

    server->SubmitResult(callHandle, returnData);
}

void RemoteFunc_Sort(IRpcServer* server, IRpcCallHandle* callHandle, const std::vector<uint8_t>& argumentData)
{
    auto returnData{argumentData};
    std::sort(returnData.begin(), returnData.end());
    std::cout << ">> Received call to '" << server->Config().name << "' with argumentData=" << argumentData
              << ", returning resultData=" << returnData << std::endl;

    server->SubmitResult(callHandle, returnData);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string configFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }

        auto ibConfig = ib::cfg::Config::FromJsonFile(configFilename);

        std::cout << "Creating ComAdapter for participant=" << participantName << " in domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);

        auto&& participantController = comAdapter->GetParticipantController();

        auto initializedPromise = std::promise<void>{};

        participantController->SetInitHandler([&initializedPromise, &participantName](auto initCmd) {
            std::cout << "Initializing " << participantName << std::endl;
            initializedPromise.set_value();
        });
        participantController->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });
        participantController->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        participantController->SetPeriod(1s);
        if (participantName == "Client")
        {
            auto exchangeFormatClientA = RpcExchangeFormat{"application/octet-stream"};
            std::map<std::string, std::string> labelsClientA{ {"KeyA", "ValA"} };
            auto clientA = comAdapter->CreateRpcClient("Add100", exchangeFormatClientA, labelsClientA, &CallReturn);

            auto exchangeFormatClientB = RpcExchangeFormat{""};
            std::map<std::string, std::string> labelsClientB{ {"KeyC", "ValC"} };
            auto clientB = comAdapter->CreateRpcClient("Sort", exchangeFormatClientB, labelsClientB, &CallReturn);

            participantController->SetSimulationTask(
                [clientA, clientB](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    Call(clientA);
                    Call(clientB);
            });
        }
        else // "Server"
        {
            auto exchangeFormatServerA = RpcExchangeFormat{"application/octet-stream"};
            std::map<std::string, std::string> labelsServerA{ {"KeyA", "ValA"}, {"KeyB", "ValB"}};
            auto serverA = comAdapter->CreateRpcServer("Add100", exchangeFormatServerA, labelsServerA, &RemoteFunc_Add100);
            
            auto exchangeFormatServerB = RpcExchangeFormat{"application/json"};
            std::map<std::string, std::string> labelsServerB{ {"KeyC", "ValC"}, {"KeyD", "ValD"}};
            auto serverB = comAdapter->CreateRpcServer("Sort", exchangeFormatServerB, labelsServerB, &RemoteFunc_Sort);

            participantController->SetSimulationTask(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(3s);
            });
        }

        auto futureState = participantController->RunAsync();

        initializedPromise.get_future().wait();
        DiscoveryResultHandler discoveryResultsHandler = [](const std::vector<RpcDiscoveryResult>& discoveryResults) {
            std::cout << ">> Found remote RpcServers:" << std::endl;
            for (const auto& entry : discoveryResults)
            {
                std::cout << "   " << entry << std::endl;
            }
        };
        comAdapter->DiscoverRpcServers("", RpcExchangeFormat{""}, {}, discoveryResultsHandler);

        auto finalState = futureState.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
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