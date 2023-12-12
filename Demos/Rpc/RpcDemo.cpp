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

#include <algorithm>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/util/serdes/Serialization.hpp"


using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::Rpc;

using namespace std::chrono_literals;

// An incrementing call counter, that is used to identify the calls of the different clients
uint16_t callCounter = 0;

static std::ostream& operator<<(std::ostream& os, const SilKit::Util::Span<const uint8_t>& v)
{
    os << "[ ";
    for (auto i : v)
        os << static_cast<int>(i) << " ";
    os << "]";
    return os;
}

static std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& v)
{
    return os << SilKit::Util::ToSpan(v);
}

void Call(IRpcClient* client)
{
    std::vector<uint8_t> argumentData{
        static_cast<uint8_t>(rand() % 10),
        static_cast<uint8_t>(rand() % 10),
        static_cast<uint8_t>(rand() % 10) };

    // Add an incrementing callCounter as userContext, to reidentify the corresponding call on reception of a 
    // call result.
    const auto userContext = reinterpret_cast<void*>(uintptr_t(callCounter++));

    // Serialize call argument data
    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(argumentData);

    client->Call(serializer.ReleaseBuffer(), userContext);
    std::cout << ">> Calling with argumentData=" << argumentData << " and userContext=" << userContext << std::endl;
}

void CallReturn(IRpcClient* /*client*/, RpcCallResultEvent event)
{
    // Deserialize call result data
    auto resultDataVector = SilKit::Util::ToStdVector(event.resultData);
    std::vector<uint8_t> resultData{};
    if (!resultDataVector.empty())
    {
        SilKit::Util::SerDes::Deserializer deserializer(resultDataVector);
        resultData = deserializer.Deserialize<std::vector<uint8_t>>();
    }
    
    switch (event.callStatus)
    {
    case RpcCallStatus::Success:
        std::cout << ">> Call " << event.userContext << " returned with resultData=" << resultData << std::endl;
        break;
    case RpcCallStatus::ServerNotReachable:
        std::cout << "Warning: Call " << event.userContext << " failed with RpcCallStatus::ServerNotReachable" << std::endl;
        break;
    case RpcCallStatus::UndefinedError:
        std::cout << "Warning: Call " << event.userContext << " failed with RpcCallStatus::UndefinedError" << std::endl;
        break;
    case RpcCallStatus::InternalServerError:
        std::cout << "Warning: Call " << event.userContext << " failed with RpcCallStatus::InternalServerError" << std::endl;
        break;
    case RpcCallStatus::Timeout:
        std::cout << "Warning: Call " << event.userContext << " failed with RpcCallStatus::Timeout" << std::endl;
        break;
    }
}

// A function offered by a RpcServer to add 100 to each enty of an array of numbers
void RemoteFunc_Add100(IRpcServer* server, RpcCallEvent event)
{
    // Deserialize call argument data
    auto argumentDataVector = SilKit::Util::ToStdVector(event.argumentData);
    SilKit::Util::SerDes::Deserializer deserializer(argumentDataVector);
    const std::vector<uint8_t> argumentData = deserializer.Deserialize<std::vector<uint8_t>>();

    // Copy argument data for calculation
    std::vector<uint8_t> resultData{argumentData};

    // Perform calculation (increment each argument value by 100)
    for (auto& v : resultData)
    {
        v += 100;
    }

    std::cout << ">> Received call with argumentData=" << argumentData << ", returning resultData=" << resultData
              << std::endl;

    // Serialize result data
    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(resultData);

    // Submit call result to client
    server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
}

// A function offered by a RpcServer to sort an array of numbers
void RemoteFunc_Sort(IRpcServer* server, RpcCallEvent event)
{
    // Deserialize call argument data
    auto argumentDataVector = SilKit::Util::ToStdVector(event.argumentData);
    SilKit::Util::SerDes::Deserializer deserializer(argumentDataVector);
    std::vector<uint8_t> argumentData = deserializer.Deserialize<std::vector<uint8_t>>();

    // Copy argument data for calculation
    std::vector<uint8_t> resultData(argumentData);

    // Perform calculation (sort argument values)
    std::sort(resultData.begin(), resultData.end());
    std::cout << ">> Received call with argumentData=" << argumentData
              << ", returning resultData=" << resultData << std::endl;

    // Serialize result data
    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(resultData);

    // Submit call result to client
    server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl
                  << "Use \"Server\" or \"Client\" as <ParticipantName>." << std::endl;
        return -1;
    }

    std::string participantName(argv[2]);
    if (participantName != "Client" && participantName != "Server")
    {
        std::cout << "Wrong participant name provided. Use either \"Client\" or \"Server\"." << std::endl;
        return 1;
    }

    std::string mediaType{SilKit::Util::SerDes::MediaTypeRpc()};
    RpcSpec rpcSpecAdd100{"Add100", mediaType};
    RpcSpec rpcSpecSort{"Sort", mediaType};

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string registryUri = "silkit://localhost:8500";

        bool runSync = true;

        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (auto arg : args)
        {
            if (arg == "--async")
            {
                runSync = false;
            }
            else
            {
                registryUri = arg;
            }
        }

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        auto operationMode = (runSync ? OperationMode::Coordinated : OperationMode::Autonomous);
        auto* lifecycleService = participant->CreateLifecycleService({operationMode});

        // Observe state changes
        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
        });
        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
        });
        lifecycleService->SetAbortHandler([](auto lastState) {
            std::cout << "Abort handler called while in state " << lastState << std::endl;
        });

        auto isClient = participantName == "Client";
        if (!isClient)
        {
            // Create RpcServer to respond to calls for "Add100"
            participant->CreateRpcServer("ServerAdd100", rpcSpecAdd100, &RemoteFunc_Add100);

            // Create RpcServer to respond to calls for "Sort"
            participant->CreateRpcServer("ServerSort", rpcSpecSort, &RemoteFunc_Sort);
        }

        // Create RpcClient to call "Add100"
        auto* clientAdd100 = (isClient ? participant->CreateRpcClient("ClientAdd100", rpcSpecAdd100, &CallReturn)
                              : nullptr);
        // Create RpcClient to call "Sort"
        auto* clientSort = (isClient ? participant->CreateRpcClient("ClientSort", rpcSpecSort, &CallReturn)
                            : nullptr);

        if (runSync)
        {
            auto* timeSyncService = lifecycleService->CreateTimeSyncService();

            if (isClient)
            {
                timeSyncService->SetSimulationStepHandler(
                    [clientAdd100, clientSort](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                        // Call both remote procedures in each simulation step
                        if (clientAdd100)
                            Call(clientAdd100);
                        if (clientSort)
                            Call(clientSort);
                    },
                    1s);
            }
            else
            {
                timeSyncService->SetSimulationStepHandler(
                    [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                        std::this_thread::sleep_for(1s);
                    },
                    1s);
            }

            auto finalStateFuture = lifecycleService->StartLifecycle();
            auto finalState = finalStateFuture.get();

            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to end the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            std::atomic<bool> isStopRequested = {false};
            std::thread workerThread;
            std::promise<void> startHandlerPromise;
            auto startHandlerFuture = startHandlerPromise.get_future();
            lifecycleService->SetCommunicationReadyHandler([&]() {
                workerThread = std::thread{[&]() {
                    startHandlerFuture.get();
                    while (lifecycleService->State() == ParticipantState::ReadyToRun ||
                           lifecycleService->State() == ParticipantState::Running)
                    {
                        if (clientAdd100)
                          Call(clientAdd100);
                        if (clientSort)
                          Call(clientSort);
                        std::this_thread::sleep_for(1s);
                    }
                    if (!isStopRequested)
                    {
                        std::cout << "Press enter to end the process..." << std::endl;
                    }
                }};
            });

            lifecycleService->SetStartingHandler([&]() {
                startHandlerPromise.set_value();
            });

            lifecycleService->StartLifecycle();
            std::cout << "Press enter to stop the simulation..." << std::endl;
            std::cin.ignore();

            isStopRequested = true;
            if (lifecycleService->State() == ParticipantState::Running || 
                lifecycleService->State() == ParticipantState::Paused)
            {
                std::cout << "User requested to stop in state " << lifecycleService->State() << std::endl;
                lifecycleService->Stop("User requested to stop");
            }

            if (workerThread.joinable())
            {
                workerThread.join();
            }
            std::cout << "The participant has shut down and left the simulation" << std::endl;
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
