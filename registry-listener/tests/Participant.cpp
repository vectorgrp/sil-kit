#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/services/orchestration/all.hpp"

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error{"Expected a registry URI"};
        }
        std::promise<void> started;
        const auto config = SilKit::Config::ParticipantConfigurationFromString(R"(
Middleware:
  EnableDomainSockets: false
Experimental:
  Metrics:
    Sinks:
      - Type: Remote
)");
        auto participant = SilKit::CreateParticipant(config, "Fixture", argv[1]);
        participant->CreateCanController("SmokeCan", "SmokeNetwork");
        auto controller = SilKit::Experimental::Participant::CreateSystemController(participant.get());
        controller->SetWorkflowConfiguration({{"Fixture"}});
        auto lifecycle = participant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});
        lifecycle->SetStartingHandler([&] { started.set_value(); });
        auto finished = lifecycle->StartLifecycle();
        if (started.get_future().wait_for(std::chrono::seconds{10}) != std::future_status::ready)
        {
            throw std::runtime_error{"Participant did not start"};
        }
        std::cout << "READY" << std::endl;
        std::string command;
        std::getline(std::cin, command);
        lifecycle->Stop("Smoke test complete");
        if (finished.wait_for(std::chrono::seconds{5}) != std::future_status::ready)
        {
            throw std::runtime_error{"Participant did not stop"};
        }
        finished.get();
    }
    catch (const std::exception& error)
    {
        std::cerr << "test participant: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
