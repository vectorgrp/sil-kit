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
    std::chrono::nanoseconds stepSizeNs{1ms};
    int64_t bandwidthKbits = 1000;
    std::chrono::nanoseconds durationSec{1s};
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
    std::chrono::nanoseconds testDurationNs = {};
    double bytesAccumulator = 0.0;
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
    if (now >= participant.testDurationNs)
    {
        gPrinter.Print("Test duration reached at " + std::to_string(now.count()) + " ns");
        participant.lifecycleService->Stop("Test duration reached");
    }
}

void OnEthernetFrameTransmitted(Participant& participant, IEthernetController* /*ctrl*/, const EthernetFrameTransmitEvent& event)
{
    //gPrinter.Print("Ethernet frame transmitted at t=" + std::to_string(event.timestamp.count()));
}



void OnSimulationStep(Participant& participant, std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    constexpr size_t minFrameSize = 63;
    double seconds = duration.count() / 1e9;
    double bytesPerStep = (participant.args.bandwidthKbits * 1000.0 / 8.0) * seconds;
    participant.bytesAccumulator += bytesPerStep;

    if (participant.bytesAccumulator >= minFrameSize)
    {
        size_t bytesToSend = static_cast<size_t>(participant.bytesAccumulator);
        bytesToSend = std::max(bytesToSend, minFrameSize);
        std::vector<uint8_t> payload(bytesToSend, 0xAB);
        if (payload.size() >= sizeof(size_t))
        {
            std::memcpy(payload.data(), &participant.frameId, sizeof(size_t));
        }
        EthernetFrame frame{payload};
        participant.ethernetController->SendFrame(frame, reinterpret_cast<void*>(&participant));
        participant.bytesSent += bytesToSend;
        participant.frameId++;
        participant.bytesAccumulator -= bytesToSend;
    }
}

auto MakeParticipant(std::string_view name, const Arguments& args)
    -> std::shared_ptr<Participant>
{
    // Setup participant configuration and registry URI
    const std::string configString = R"(
Logging:
  Sinks:
    - Type: Stdout
      Level: Error
)";
    auto p = std::make_shared<Participant>();
    auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);
    auto participant = SilKit::CreateParticipant(participantConfiguration, std::string{name}, args.registryUri);

    // Create EthernetController
    auto* ethernetController = participant->CreateEthernetController("EthernetController1", "ETH1");

    // Register handlers
    ethernetController->AddFrameTransmitHandler([p](auto&& ctrl, auto&& event) {
        OnEthernetFrameTransmitted(*p, ctrl, event);
    });
    ethernetController->AddFrameHandler([p](auto&& ctrl, auto&& event) {
        OnEthernetFrameReceived(*p, ctrl, event);
    });

    auto* lifecycleService = participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    p->lifecycleService = lifecycleService;
    p->timeSyncService = timeSyncService;

    // Register simulation step handler
    timeSyncService->SetSimulationStepHandler(
        [p](auto now, auto duration) {
            OnSimulationStep(*p, now, duration);
        },
        args.stepSizeNs);

    lifecycleService->SetCommunicationReadyHandler([ethernetController]() {
        ethernetController->Activate();
    });


    p->participant = std::move(participant);
    p->ethernetController = ethernetController;
    p->args = args;
    p->bytesSent = 0;
    p->frameId = 0;
    p->testDurationNs = args.durationSec;
    p->finalStateFuture = lifecycleService->StartLifecycle();

    return p;
}

std::chrono::nanoseconds ParseDurationToNanoseconds(const std::string& str)
{
    size_t pos = 0;
    while (pos < str.size() && std::isdigit(str[pos])) ++pos;
    std::string numPart = str.substr(0, pos);
    std::string suffix = str.substr(pos);
    int64_t value = numPart.empty() ? 0 : std::stoll(numPart);
    if (suffix == "ns")
        return std::chrono::nanoseconds(value);
    else if (suffix == "us" || suffix.empty())
        return std::chrono::microseconds(value);
    else if (suffix == "ms")
        return std::chrono::milliseconds(value);
    else if (suffix == "s" || suffix.empty())
        return std::chrono::seconds(value);
    else
        throw std::runtime_error{"Cannot parse time string '" + str + "'"};
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
        "step-size", "s", "1ms", "[--step-size <NANOSECONDS>]",
        std::vector<std::string>{"Simulation step size in [ms,ns,s] defaulting to seconds (default: 1000000ns, i.e. 1ms)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "duration", "d", "1s", "[--duration <SECONDS>]",
        std::vector<std::string>{"Simulation duration in [ms,ns,s] defaulting to seconds [s] (default: 1ms)"});
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
    args.stepSizeNs = ParseDurationToNanoseconds(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("step-size").Value());
    auto durationStr = cmdParser.Get<SilKit::Util::CommandlineParser::Option>("duration").Value();
    args.durationSec = ParseDurationToNanoseconds(durationStr);

    auto&& config = SilKit::Config::ParticipantConfigurationFromString("{}");
    auto&& registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
    args.registryUri = registry->StartListening("silkit://localhost:0"); // get actual listening address


    auto wallclockStart = std::chrono::high_resolution_clock::now();

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

    for (auto& participant : participants)
    {
        participant->finalStateFuture.get();
    }

    auto wallclockEnd = std::chrono::high_resolution_clock::now();
    auto wallclockDuration = std::chrono::duration_cast<std::chrono::duration<double>>(wallclockEnd - wallclockStart);
    gPrinter.Print("Bandwidth Test Tool Parameters:");
    gPrinter.Print("  number-of-participants: " + std::to_string(numberOfParticipants));
    gPrinter.Print("  bandwidth: " + std::to_string(args.bandwidthKbits) + " KBit/s");
    gPrinter.Print("  step-size: " + std::to_string(args.stepSizeNs.count()) + " ns");
    gPrinter.Print("  duration: " + durationStr + " (" + std::to_string(args.durationSec.count()) + " ns)");
    gPrinter.Print("  run time: " + std::to_string(wallclockDuration.count()) + "s");

    return 0;
}
