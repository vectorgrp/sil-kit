// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// A live terminal dashboard of a SIL Kit simulation. It joins the simulation as a passive participant and
// uses the experimental public service discovery to show all participants, their services and how they are
// interconnected: bus networks with their controllers, pub/sub topics and RPC functions with their matches,
// and network simulators. Every change is highlighted in the topology and recorded in an event log.
//
// With --sim-time the observer additionally takes part in the virtual time synchronization (autonomous
// lifecycle) and shows the global simulation time it is granted. This makes the observer an active
// participant; see Session below for how it avoids disturbing the simulation.

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/experimental/serviceDiscovery/IServiceDiscovery.hpp"

#include "SignalHandler.hpp"
#include "CommandlineParser.hpp"

#include "Terminal.hpp"
#include "NetworkModel.hpp"
#include "Dashboard.hpp"

using namespace std::chrono_literals;
using SilKit::Util::CommandlineParser;

namespace {

std::atomic<bool> gStopRequested{false};

// Removed elements stay visible (struck through) for this long before they vanish from the dashboard.
constexpr auto GhostDuration = 4s;

// Once the last synchronized participant is gone for this long, the observer rejoins the time sync afresh.
constexpr auto TimeSyncIdleTimeout = 2s;

namespace Orchestration = SilKit::Services::Orchestration;

struct SessionSettings
{
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    std::string participantName;
    std::string registryUri;
    bool simulationTime{false};
    std::chrono::nanoseconds stepSize{10ms};
};

//! One SIL Kit participant of the observer, feeding the model.
//!
//! With simulation time enabled the participant runs an autonomous lifecycle with virtual time synchronization,
//! and its step handler reports the granted time. Two rules keep it from disturbing the simulation:
//!  - It never advances alone: a step is only completed while another synchronized participant exists.
//!    Otherwise its clock would run ahead and participants joining later would hop on at that time (a
//!    coordinated participant joining an already advanced time even aborts the simulation).
//!  - When the time sync becomes idle (all synchronized participants left), the session is replaced by a
//!    fresh one starting at time zero, ready for the next simulation.
class Session
{
public:
    Session(Introspection::NetworkModel& model, const SessionSettings& settings)
        : _model{model}
        , _active{std::make_shared<std::atomic<bool>>(true)}
    {
        _participant =
            SilKit::CreateParticipant(settings.configuration, settings.participantName, settings.registryUri);

        // Handlers of a session that is being replaced must not touch the model anymore: its teardown would
        // otherwise report every peer as disconnected.
        const auto active = _active;
        auto* systemMonitor = _participant->CreateSystemMonitor();
        systemMonitor->SetParticipantConnectedHandler(
            [&model, active](const Orchestration::ParticipantConnectionInformation& info) {
            if (*active)
            {
                model.OnParticipantConnected(info.participantName);
            }
        });
        systemMonitor->SetParticipantDisconnectedHandler(
            [&model, active](const Orchestration::ParticipantConnectionInformation& info) {
            if (*active)
            {
                model.OnParticipantDisconnected(info.participantName);
            }
        });
        systemMonitor->AddSystemStateHandler([&model, active](Orchestration::SystemState state) {
            if (*active)
            {
                model.OnSystemState(state);
            }
        });
        systemMonitor->AddParticipantStatusHandler(
            [&model, active](const Orchestration::ParticipantStatus& status) {
            if (*active)
            {
                model.OnParticipantStatus(status.participantName, status.state);
            }
        });

        auto* serviceDiscovery = SilKit::Experimental::Participant::CreateServiceDiscovery(_participant.get());
        serviceDiscovery->SetServiceDiscoveryHandler(
            [&model, active](SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryEventType type,
                             const SilKit::Experimental::ServiceDiscovery::ServiceDescriptor& descriptor) {
            if (*active)
            {
                model.OnServiceEvent(type, descriptor);
            }
        });

        if (settings.simulationTime)
        {
            _lifecycle = _participant->CreateLifecycleService({Orchestration::OperationMode::Autonomous});
            _timeSync = _lifecycle->CreateTimeSyncService();
            _timeSync->SetSimulationStepHandlerAsync(
                [this, active](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                if (!*active)
                {
                    return;
                }
                _model.OnSimulationStep(now);
                if (now > 0ns)
                {
                    _advanced = true;
                }
                _stepPending = true;
                CompletePendingStep();
            }, settings.stepSize);
            _done = _lifecycle->StartLifecycle();
        }
    }

    ~Session()
    {
        *_active = false;
        if (_lifecycle != nullptr)
        {
            const auto state = _lifecycle->State();
            if (state == Orchestration::ParticipantState::Running || state == Orchestration::ParticipantState::Paused)
            {
                _lifecycle->Stop("Observer leaves the time synchronization");
            }
            if (_done.valid())
            {
                _done.wait_for(2s);
            }
        }
    }

    //! Called periodically from the UI thread. Returns false if this session should be replaced.
    auto Poll() -> bool
    {
        if (_timeSync == nullptr)
        {
            return true;
        }
        const bool peers = _model.HasSynchronizedPeers();
        _model.SetSimulationTimeWaiting(!peers);
        if (peers)
        {
            _lastPeerSeen = std::chrono::steady_clock::now();
            CompletePendingStep();
            return true;
        }
        return !(_advanced && std::chrono::steady_clock::now() - _lastPeerSeen > TimeSyncIdleTimeout);
    }

private:
    void CompletePendingStep()
    {
        if (_model.HasSynchronizedPeers() && _stepPending.exchange(false))
        {
            _timeSync->CompleteSimulationStep();
        }
    }

    Introspection::NetworkModel& _model;
    std::shared_ptr<std::atomic<bool>> _active;
    Orchestration::ILifecycleService* _lifecycle{nullptr};
    Orchestration::ITimeSyncService* _timeSync{nullptr};
    std::future<Orchestration::ParticipantState> _done;
    std::atomic<bool> _stepPending{false};
    std::atomic<bool> _advanced{false};
    std::chrono::steady_clock::time_point _lastPeerSeen{std::chrono::steady_clock::now()};
    // Declared last so it is destroyed first: its threads may still run the handlers above.
    std::unique_ptr<SilKit::IParticipant> _participant;
};

//! Keeps an observer session alive and replaces it when it asks for it. Polled by the UI loops.
class SessionKeeper
{
public:
    SessionKeeper(Introspection::NetworkModel& model, SessionSettings settings)
        : _model{model}
        , _settings{std::move(settings)}
    {
        if (_settings.simulationTime)
        {
            _model.EnableSimulationTime(_settings.stepSize);
        }
        _session = std::make_unique<Session>(_model, _settings);
    }

    void Poll()
    {
        if (!_session->Poll())
        {
            _session.reset();
            _model.ResetSimulationTime();
            _session = std::make_unique<Session>(_model, _settings);
        }
    }

private:
    Introspection::NetworkModel& _model;
    SessionSettings _settings;
    std::unique_ptr<Session> _session;
};

void RunDashboard(Introspection::NetworkModel& model, SessionKeeper& sessions, Introspection::Terminal& terminal,
                  const std::string& registryUri, const std::string& participantName)
{
    Introspection::Dashboard dashboard{registryUri, participantName};
    terminal.EnterFullscreen();

    auto lastSize = Introspection::Terminal::Size();
    while (!gStopRequested)
    {
        const auto size = Introspection::Terminal::Size();
        if (size != lastSize)
        {
            terminal.Invalidate();
            lastSize = size;
        }
        sessions.Poll();
        const auto state = model.Snapshot(GhostDuration);
        terminal.DrawFrame(dashboard.Render(state, size.first, size.second), size.first);
        std::this_thread::sleep_for(100ms);
    }
    terminal.LeaveFullscreen();
}

void RunPlainLog(Introspection::NetworkModel& model, SessionKeeper& sessions, bool color)
{
    uint64_t lastPrinted = 0;
    while (!gStopRequested)
    {
        sessions.Poll();
        const auto state = model.Snapshot(GhostDuration);
        for (const auto& event : state.log)
        {
            if (event.sequence > lastPrinted)
            {
                std::cout << Introspection::FormatLogEvent(event).Render(color) << std::endl;
                lastPrinted = event.sequence;
            }
        }
        std::this_thread::sleep_for(100ms);
    }
}

} // namespace

int main(int argc, char** argv)
{
    CommandlineParser parser;
    parser.SetDescription("Live terminal dashboard of the participants, services and topology of a SIL Kit "
                          "simulation, based on the experimental service discovery.");
    parser.Add<CommandlineParser::Flag>("help", "h", "-h, --help", std::vector<std::string>{"Get this help."});
    parser.Add<CommandlineParser::Option>("name", "n", "SilKitObserver", "-n, --name <name>",
                                          std::vector<std::string>{"The participant name of the observer.",
                                                                   "Defaults to 'SilKitObserver'."});
    parser.Add<CommandlineParser::Option>("registry-uri", "u", "silkit://localhost:8500", "-u, --registry-uri <uri>",
                                          std::vector<std::string>{"The registry URI to connect to.",
                                                                   "Defaults to 'silkit://localhost:8500'."});
    parser.Add<CommandlineParser::Option>(
        "config", "c", "", "-c, --config <filePath>",
        std::vector<std::string>{"Path to the participant configuration YAML or JSON file.",
                                 "Logging to stdout should be avoided, it interferes with the dashboard."});
    parser.Add<CommandlineParser::Flag>(
        "plain", "p", "-p, --plain",
        std::vector<std::string>{"Do not draw the dashboard, only print the event log line by line.",
                                 "Used automatically if stdout is not an interactive terminal."});
    parser.Add<CommandlineParser::Flag>("no-color", "", "--no-color",
                                        std::vector<std::string>{"Print the plain event log without colors."});
    parser.Add<CommandlineParser::Flag>(
        "sim-time", "t", "-t, --sim-time",
        std::vector<std::string>{"Show the global simulation time. The observer then takes part in the virtual",
                                 "time synchronization with an autonomous lifecycle (it is no longer passive)."});
    parser.Add<CommandlineParser::Option>(
        "sim-step", "s", "10", "-s, --sim-step <milliseconds>",
        std::vector<std::string>{"Step size of the observer's time synchronization, i.e. the resolution of the",
                                 "simulation time. Larger steps cost the simulation less. Defaults to 10."});

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
        parser.PrintUsageInfo(std::cerr);
        return -1;
    }
    if (parser.Get<CommandlineParser::Flag>("help").Value())
    {
        parser.PrintUsageInfo(std::cout);
        return 0;
    }

    const auto participantName = parser.Get<CommandlineParser::Option>("name").Value();
    const auto registryUri = parser.Get<CommandlineParser::Option>("registry-uri").Value();
    const auto configPath = parser.Get<CommandlineParser::Option>("config").Value();
    const bool simulationTime = parser.Get<CommandlineParser::Flag>("sim-time").Value();
    std::chrono::nanoseconds stepSize{0};
    try
    {
        stepSize = std::chrono::milliseconds{std::stoul(parser.Get<CommandlineParser::Option>("sim-step").Value())};
    }
    catch (const std::exception&)
    {
    }
    if (stepSize <= 0ns)
    {
        std::cerr << "Error: --sim-step expects a positive number of milliseconds" << std::endl;
        return -1;
    }

    Introspection::Terminal terminal;
    const bool interactive = terminal.EnableVirtualTerminal();
    const bool plain = parser.Get<CommandlineParser::Flag>("plain").Value() || !interactive;
    const bool color = interactive && !parser.Get<CommandlineParser::Flag>("no-color").Value();

    // The model must outlive the participant: its handlers may fire until the participant is destroyed.
    Introspection::NetworkModel model{participantName};

    try
    {
        // Without a configuration the participant has no log sinks, keeping the terminal clean.
        auto configuration = configPath.empty() ? SilKit::Config::ParticipantConfigurationFromString("")
                                                : SilKit::Config::ParticipantConfigurationFromFile(configPath);

        std::cout << "Connecting to " << registryUri << " as '" << participantName << "' ..." << std::endl;
        SessionKeeper sessions{model, SessionSettings{std::move(configuration), participantName, registryUri,
                                                      simulationTime, stepSize}};

        RegisterSignalHandler([](int) { gStopRequested = true; });

        if (plain)
        {
            std::cout << "Observing the simulation, press Ctrl-C to quit." << std::endl;
            RunPlainLog(model, sessions, color);
        }
        else
        {
            RunDashboard(model, sessions, terminal, registryUri, participantName);
        }
        ShutdownSignalHandler();
    }
    catch (const std::exception& error)
    {
        terminal.LeaveFullscreen();
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -2;
    }

    return 0;
}
