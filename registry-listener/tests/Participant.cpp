// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <limits>
#include <locale>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/RegistryEventListener.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/services/orchestration/all.hpp"

namespace {

class GroupedNumbers : public std::numpunct<char>
{
    auto do_grouping() const -> std::string override { return "\3"; }
    auto do_thousands_sep() const -> char override { return ','; }
};

void EmitEvents()
{
    std::locale::global(std::locale{std::locale::classic(), new GroupedNumbers});
    std::string text = "\"\\";
    for (char ch = 0; ch < 0x20; ++ch)
    {
        text += ch;
    }
    text += "\xc3\xa9";
    const auto generation = std::numeric_limits<uint64_t>::max();
    RegistryEventListener listener;
    listener.OnLoggerCreated(nullptr);
    listener.OnRegistryUri(text);
    listener.OnGenerationCreated(text, generation);
    listener.OnGenerationStateChanged(text, generation, SilKit::Services::Orchestration::SystemState::Running);
    listener.OnGenerationRemoved(text, generation);
    listener.OnParticipantConnected(text, text, generation);
    listener.OnParticipantDisconnected(text, text, generation);
    const std::vector<std::string> required{text, "", "123"};
    listener.OnRequiredParticipantsUpdate(text, text, required, generation);
    listener.OnRequiredParticipantsUpdate("", "", {}, std::nullopt);

    SilKit::Services::Orchestration::ParticipantStatus status;
    status.state = SilKit::Services::Orchestration::ParticipantState::Running;
    status.enterReason = text;
    status.enterTime = std::chrono::system_clock::time_point{std::chrono::milliseconds{-123}};
    status.refreshTime = std::chrono::system_clock::time_point{std::chrono::milliseconds{456}};
    listener.OnParticipantStatusUpdate(text, text, status, generation);

    SilKit::Core::Discovery::ServiceDiscoveryEvent service;
    service.type = SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated;
    service.serviceDescriptor.SetParticipantNameAndComputeId(text);
    service.serviceDescriptor.SetServiceId(generation);
    service.serviceDescriptor.SetServiceName(text);
    service.serviceDescriptor.SetNetworkName(text);
    service.serviceDescriptor.SetSupplementalData({{text, text}, {"", ""}, {"123", "123"}});
    listener.OnServiceDiscoveryEvent(text, text, service);
    listener.OnMetricsUpdate(text, text, {{{1, text, VSilKit::MetricKind::ATTRIBUTE, text},
                                         {2, "counter", VSilKit::MetricKind::COUNTER, "18446744073709551615"},
                                         {3, "statistic", VSilKit::MetricKind::STATISTIC, "[1,2,3]"},
                                         {4, "peers", VSilKit::MetricKind::STRING_LIST, "[\"123\"]"}}});
    listener.OnMetricsUpdate("", "", {});
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        if (argc == 2 && std::string{argv[1]} == "--emit-events")
        {
            EmitEvents();
            return 0;
        }
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
