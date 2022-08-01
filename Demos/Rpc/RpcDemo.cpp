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
#include <sstream>
#include <thread>
#include <algorithm>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"


using namespace SilKit::Services::Rpc;
using namespace std::chrono_literals;

std::chrono::milliseconds callReturnTimeout{ 5000ms };

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

    const auto userContext = reinterpret_cast<void*>(uintptr_t(rand()));

    client->Call(argumentData, userContext);

    std::cout << "<< Calling with argumentData=" << argumentData << " and userContext=" << userContext << std::endl;
}

void CallReturn(IRpcClient* /*cbClient*/, RpcCallResultEvent event)
{
    switch (event.callStatus)
    {
    case RpcCallStatus::Success:
        std::cout << ">> Call " << event.userContext << " returned with resultData=" << event.resultData << std::endl;
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
    }
}

void RemoteFunc_Add100(IRpcServer* server, RpcCallEvent event)
{
    auto returnData = SilKit::Util::ToStdVector(event.argumentData);
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
    auto returnData = SilKit::Util::ToStdVector(event.argumentData);
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
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl
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

        auto* lifecycleService = participant->CreateLifecycleService();
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        lifecycleService->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });
        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        if (participantName == "Client")
        {
            SilKit::Services::Rpc::RpcClientSpec dataSpecAdd100{"Add100", "application/octet-stream"};
            dataSpecAdd100.AddLabel("KeyA", "ValA");
            auto clientA = participant->CreateRpcClient("ClientCtrl1", dataSpecAdd100, &CallReturn);

            SilKit::Services::Rpc::RpcClientSpec dataSpecSort{"Sort", "application/octet-stream"};
            dataSpecAdd100.AddLabel("KeyC", "ValC");
            auto clientB =
                participant->CreateRpcClient("ClientCtrl2", dataSpecSort, &CallReturn);


            timeSyncService->SetSimulationStepHandler(
                [clientA, clientB](std::chrono::nanoseconds now,
                                                                           std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    Call(clientA);
                    Call(clientB);
                }, 1s);
        }
        else // "Server"
        {
            SilKit::Services::Rpc::RpcServerSpec dataSpecAdd100{"Add100", "application/octet-stream"};
            dataSpecAdd100.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred);
            dataSpecAdd100.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Preferred);

            participant->CreateRpcServer("ServerCtrl1", dataSpecAdd100, &RemoteFunc_Add100);

            SilKit::Services::Rpc::RpcServerSpec dataSpecSort{"Sort", "application/octet-stream"};
            dataSpecSort.AddLabel("KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Preferred);
            dataSpecSort.AddLabel("KeyD", "ValD", SilKit::Services::MatchingLabel::Kind::Preferred);
            participant->CreateRpcServer("ServerCtrl2", dataSpecSort, &RemoteFunc_Sort);

            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(3s);
            }, 1s);
        }

        auto lifecycleFuture = lifecycleService->StartLifecycle({true});
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
