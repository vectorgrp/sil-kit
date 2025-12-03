// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <memory>
#include <string_view>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/Services/Orchestration/ILifecycleService.hpp"
#include "silkit/Services/Orchestration/ITimeSyncService.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "../../communication/include/CommandlineParser.hpp"

using namespace SilKit::Services::Ethernet;
using namespace std::chrono_literals;

struct Participant; //fwd

struct Arguments
{
    int stepSizeNs = 1000000;
    int bandwidthKbits = 1000;
    int durationSec = 1;
    std::string registryUri = "silkit://localhost:0";
};

struct Participant
{
    std::unique_ptr<SilKit::IParticipant> participant{};
    SilKit::Services::Ethernet::IEthernetController* ethernetController{};
    SilKit::Services::Orchestration::ILifecycleService* lifecycleService{};
    SilKit::Services::Orchestration::ITimeSyncService* timeSyncService{};
    std::future<SilKit::Services::Orchestration::ParticipantState> finalStateFuture;
    Arguments args;
    size_t bytesSent = 0;
    size_t frameId = 0;
    size_t bytesReceived = 0;
    std::chrono::nanoseconds firstReceiveTime = {};
    std::chrono::nanoseconds lastReceiveTime = {};
    std::chrono::nanoseconds lastPrintTime = {};
};


class Printer
{
public:
    Printer()
        : _running(true), _thread([this] { this->Run(); })
    {
    }
    ~Printer()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _running = false;
        }
        _cv.notify_one();
        if (_thread.joinable())
            _thread.join();
    }
    void Print(const std::string& msg)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(msg);
        }
        _cv.notify_one();
    }
private:
    void Run()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return !_queue.empty() || !_running; });
            while (!_queue.empty())
            {
                std::cout << _queue.front() << std::endl;
                _queue.pop();
            }
            if (!_running && _queue.empty())
                break;
        }
    }
    std::queue<std::string> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _running;
    std::thread _thread;
};

static Printer gPrinter;

void OnEthernetFrameReceived(Participant& participant, IEthernetController* /*ctrl*/, const EthernetFrameEvent& event)
{
    size_t frameSize = event.frame.raw.size();
    participant.bytesReceived += frameSize;
    auto now = event.timestamp;
    if (participant.firstReceiveTime == std::chrono::nanoseconds{})
    {
        participant.firstReceiveTime = now;
    }
    participant.lastReceiveTime = now;

    auto elapsed = participant.lastReceiveTime - participant.firstReceiveTime;
    double elapsedSeconds = elapsed.count() / 1e9;
    double bandwidthKbits = 0.0;
    if (elapsedSeconds > 0.0)
    {
        bandwidthKbits = (participant.bytesReceived * 8.0) / 1000.0 / elapsedSeconds;
    }
    double errorMargin = 0.0;
    if (participant.args.bandwidthKbits > 0)
    {
        errorMargin = (bandwidthKbits - participant.args.bandwidthKbits) / participant.args.bandwidthKbits * 100.0;
    }
    // Print only every second
    if (now - participant.lastPrintTime >= std::chrono::seconds(1))
    {
        gPrinter.Print("[RECV] Bandwidth: " + std::to_string(bandwidthKbits) + " Kbit/s, Error: " + std::to_string(errorMargin) + "%");
        participant.lastPrintTime = now;
    }
}

void OnEthernetFrameTransmitted(IEthernetController* /*ctrl*/, const EthernetFrameTransmitEvent& event)
{
    //gPrinter.Print("Ethernet frame transmitted at t=" + std::to_string(event.timestamp.count()));
}



void OnSimulationStep(Participant& participant, std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    // Calculate bytes to send for this step
    double seconds = duration.count() / 1e9;
    size_t bytesPerStep = static_cast<size_t>((participant.args.bandwidthKbits * 1000 / 8) * seconds);
    if (bytesPerStep == 0) return;

    // Send frames (one frame per step, or split into multiple if needed)
    std::vector<uint8_t> payload(bytesPerStep, 0xAB); // Example payload
    // Optionally add frameId or timestamp to payload
    if (payload.size() >= sizeof(size_t))
    {
        std::memcpy(payload.data(), &participant.frameId, sizeof(size_t));
    }
    EthernetFrame frame{payload};
    participant.ethernetController->SendFrame(frame, reinterpret_cast<void*>(&participant));
    participant.bytesSent += bytesPerStep;
    participant.frameId++;

}

auto MakeParticipant(std::string_view name, const Arguments& args)
    -> std::shared_ptr<Participant>
{
    // Setup participant configuration and registry URI
    const std::string configString = R"(
Logging:
  Sinks:
    - Type: Stdout
      Level: Info
)";
    auto p = std::make_shared<Participant>();
    auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);
    auto participant = SilKit::CreateParticipant(participantConfiguration, std::string{name}, args.registryUri);

    // Create EthernetController
    auto* ethernetController = participant->CreateEthernetController("EthernetController1", "ETH1");

    // Register handlers
    ethernetController->AddFrameTransmitHandler(OnEthernetFrameTransmitted);
    ethernetController->AddFrameHandler([p](IEthernetController* ctrl, const EthernetFrameEvent& event) {
        OnEthernetFrameReceived(*p, ctrl, event);
    });

    auto* lifecycleService = participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    p->participant = std::move(participant);
    p->ethernetController = ethernetController;
    p->lifecycleService = lifecycleService;
    p->timeSyncService = timeSyncService;
    p->args = args;
    p->bytesSent = 0;
    p->frameId = 0;

    // Register simulation step handler
    timeSyncService->SetSimulationStepHandler(
        [p](auto now, auto duration) {
            OnSimulationStep(*p, now, duration);
        },
        std::chrono::nanoseconds{args.stepSizeNs});

    lifecycleService->SetCommunicationReadyHandler([ethernetController]() {
        ethernetController->Activate();
    });

    p->finalStateFuture = lifecycleService->StartLifecycle();

    return p;
}

int main(int argc, char** argv)
{
    SilKit::Util::CommandlineParser cmdParser;
    cmdParser.SetDescription("Bandwidth Demo: Simulate multiple participants and measure bandwidth.");
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "number-of-participants", "n", "2", "[--number-of-participants <N>]",
        std::vector<std::string>{"Number of participants to create (default: 1)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "bandwidth", "b", "1000", "[--bandwidth <VALUE>]",
        std::vector<std::string>{"Bandwidth value to use KBit/s (default: 1000)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "step-size", "s", "1000000", "[--step-size <NANOSECONDS>]",
        std::vector<std::string>{"Simulation step size in nanoseconds (default: 1000000, i.e. 1ms)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "duration", "d", "1", "[--duration <SECONDS>]",
        std::vector<std::string>{"Simulation duration in seconds (default: 1)"});
    try {
        cmdParser.ParseArguments(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cmdParser.PrintUsageInfo(std::cout);
        return -1;
    }
    int numberOfParticipants = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("number-of-participants").Value());
    Arguments args;
    args.bandwidthKbits = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("bandwidth").Value());
    args.stepSizeNs = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("step-size").Value());
    args.durationSec = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("duration").Value());

    auto&& config = SilKit::Config::ParticipantConfigurationFromString("{}");
    auto&& registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
    args.registryUri = registry->StartListening("silkit://localhost:0"); // get actual listening address

    std::vector<std::string> participantNames;
    std::vector<std::shared_ptr<Participant>> participants;
    for (auto i = 0; i < numberOfParticipants; i++)
    {
        auto&& name = "Participant_" + std::to_string(i);
        participantNames.push_back(name);
        participants.emplace_back(MakeParticipant(name, args));
    }

    auto&& firstParticipant = participants.at(0);
    auto&& systemController =
        SilKit::Experimental::Participant::CreateSystemController(firstParticipant->participant.get());
    systemController->SetWorkflowConfiguration({participantNames});

    // Sleep for the requested duration before waiting for lifecycle completion
    gPrinter.Print("Main thread sleeping for " + std::to_string(args.durationSec) + " seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(args.durationSec));

    firstParticipant->lifecycleService->Stop("Test stopped.");
    for (auto& participant : participants)
    {
        participant->finalStateFuture.get();
    }

    return 0;
}
