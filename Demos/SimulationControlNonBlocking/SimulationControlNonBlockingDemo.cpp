// Copyright (c) Vector Informatik GmbH. All rights reserved.

// This Demo is a test of the simulation pause/continue controls.
// The Demo consists of three participants that will be paused
// by an external worker thread by calling the ParticipantController's Pause/Continue methods.
// The participant S1 will invoke the Pause() call from within it's SimTask.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <algorithm>
#include <random>
#include <cctype>

#include "ib/IntegrationBus.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/cfg/string_utils.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include "ib/extensions/CreateExtension.hpp"

using namespace ib::mw;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

static auto randomDelay() -> std::chrono::milliseconds
{
    static std::random_device _dev;
    static std::mt19937 _twister{_dev()};
    std::normal_distribution<> dist(50, 2); // centered around 50ms
    int64_t count{static_cast<std::chrono::milliseconds::rep>(dist(_twister))};
    return std::chrono::milliseconds{count};
}

bool done = false;
const std::chrono::milliseconds simulationPeriod{1};
const std::chrono::milliseconds expectedTimeStep{simulationPeriod};
std::promise<void> isStarted;

// Thread safe printing:
std::mutex printMutex;
#define PRINT(...) do { \
        {\
            std::lock_guard<std::mutex> lock{ printMutex };\
            std::cout << __VA_ARGS__ << std::endl;\
        }\
    } while(false)

//print timestamps in milliseconds
std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds ns)
{
    auto ms = std::chrono::duration_cast<
        std::chrono::duration<double, std::milli>>(ns);
    return out << (std::to_string(ms.count()) + "ms");
}

static const int g_domainId = 42;

class Participant
{
public:
    Participant(const ib::cfg::Config& config, const ib::cfg::Participant& participantCfg)
        : _config{config}
        , _participantName{participantCfg.name}
        , _participantConfig{ participantCfg }
    {
        Setup();
    }
    // Participant's simulation controls
    void Pause(const std::string& reason)
    {
        _comAdapter->GetParticipantController()->Pause(reason);
    }
    void PauseInSimtask()
    {
        _pauseInSimtask = true;
        auto paused = _paused.get_future();
        auto ok = paused.wait_for(5s);
        if (ok != std::future_status::ready)
        {
            PRINT("    pauseInSimTask " << _participantName << " NOT paused?!");
        }
        _paused = std::promise<void>();
    }

    void Continue()
    {
        _comAdapter->GetParticipantController()->Continue();
    }

    const std::string& Name() const
    {
        return _participantName;
    }

    ib::mw::sync::ParticipantState State() const
    {
        return _comAdapter->GetParticipantController()->State();
    }

    void CompleteSimTask()
    {
        auto&& participantController = _comAdapter->GetParticipantController();
        participantController->CompleteSimulationTask();
    }

    void WaitForSimTask()
    {
      std::unique_lock<std::mutex> lock(_simTaskCvMutex);
      while (!_simTaskHasRun)
      {
        _simTaskCv.wait_for(lock, 10ms);
      }
      _simTaskHasRun = false;
    }
private:
    void Setup()
    {
        _comAdapter = ib::CreateComAdapter(_config, _participantName, g_domainId);
        auto&& participantController = _comAdapter->GetParticipantController();
        auto&& monitor = _comAdapter->GetSystemMonitor();

        // Participants starting with the letter 'P' will be our publishers
        if (_participantName == "P1")
        {
                // initialize the Publisher P1 which will also serve as a system state coordinator
            monitor->RegisterSystemStateHandler(
                std::bind(&Participant::OnSystemStateChanged, this, std::placeholders::_1));

            auto&& topic = _participantConfig.genericPublishers.at(0).name;
            auto&& publisher = _comAdapter->CreateGenericPublisher(topic);
            _simTaskHasRun = false;
            participantController->SetSimulationTaskAsync([this, publisher](auto now, auto period) {
              try {
                // slow down demo output a bit
                std::this_thread::sleep_for(50ms);

                auto msg = std::to_string(now.count());
                PRINT(">>> simtask " << _participantName << " @ " << now.count() << "ms sent msg=" << msg);
                publisher->Publish({ msg.begin(), msg.end() });

                Validate(now);

                // signal the controlThread we're done with this step
                std::unique_lock<std::mutex> lock(_simTaskCvMutex);
                _simTaskHasRun = true;
                _simTaskCv.notify_all();
              }
              catch (const std::exception& e)
              {
                std::cerr << e.what() << std::endl;
              }

            });
        }
        else
        {
            // subscribers only print messages and react to Pause/Continue signals
            auto&& topic = _participantConfig.genericSubscribers.at(0).name;
            auto&& subscriber = _comAdapter->CreateGenericSubscriber(topic);
            subscriber->SetReceiveMessageHandler(
                [this](auto&& subscriber, auto&& data) {
                    this->OnReceive(subscriber, data);
                });
            participantController->SetSimulationTask([this](auto now) {
                Validate(now);

                PRINT("    simtask " << _participantName << " @ " << now.count() << "ms");

                if (_pauseInSimtask)
                {
                    _pauseInSimtask = false;
                    _comAdapter->GetParticipantController()->Pause("Pause inside of SimTask of " + _participantName);
                    _paused.set_value();
                }

            });
        }
        // monitor each participants state updates:
        monitor->RegisterParticipantStatusHandler(
            std::bind(&Participant::OnParticipantStatus, this, std::placeholders::_1));

        // start IB simulation in background thread
        _finalState = participantController->RunAsync();
    }

    void OnReceive(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
    {
        std::string msg{ data.begin(), data.end() };
        auto now = _comAdapter->GetParticipantController()->Now();
        PRINT("    receive " << _participantName << " @ " << now.count()
            << "ms msg=" << msg);
        ValidateOnReceive(subscriber->Config().name, msg);
    }
    void OnSystemStateChanged(sync::SystemState state)
    {
        // Ensure simulation is started in a controlled way by the publisher
        auto&& systemController = _comAdapter->GetSystemController();
        switch (state)
        {
        case sync::SystemState::Idle:
            for (auto&& participant : _config.simulationSetup.participants)
            {
                systemController->Initialize(participant.name);
            }
            break;
        case sync::SystemState::Initialized:
            PRINT("### " << _participantName << " Sending Run command ");
            systemController->Run();
            isStarted.set_value();
            break;
        case sync::SystemState::Stopped:
            systemController->Shutdown();
            break;
        case sync::SystemState::Paused:
            PRINT("### SystemState Paused");
            break;
        case sync::SystemState::Running:
            PRINT("### SystemState Running");
            break;
        default:
          break;
        }

    }
    void Validate(std::chrono::nanoseconds simTaskNow)
    {
        if (_validation.lastSimTaskTime > std::chrono::nanoseconds::min())
        {
            auto delta = simTaskNow  - _validation.lastSimTaskTime;
            if (delta > expectedTimeStep)
            //if((_validation.lastSimTaskTime + simulationPeriod) != simTaskNow)
            {
                std::cout << _participantName << std::endl;
                PRINT("    XXXXXXX " << _participantName << " UNEXPECTED SIMTASK TIME: "
                    << " now=" << simTaskNow
                    << " lastSimTaskTime=" << _validation.lastSimTaskTime
                    << " delta=" << delta
                );
                throw std::runtime_error{"Validation inside of simTask failed delta=" + std::to_string(delta.count())};
            }
        }
        _validation.lastSimTaskTime = simTaskNow;

    }
    void ValidateOnReceive(const std::string& topic,  const std::string& msgStr)
    {
        auto now = _comAdapter->GetParticipantController()->Now();
        std::stringstream parser{ msgStr };
        uint64_t msgInt;
        parser >> msgInt;
        auto msg = Nanoseconds{ msgInt };
        if (_validation.lastMessage > Nanoseconds::min())
        {
            auto delta = msg  - _validation.lastMessage;
            if (delta > expectedTimeStep)
            {
                PRINT("    XXXXXXX " << _participantName << " UNEXPECTED MESSAGE: " 
                    << " Previous Message Contents=" << _validation.lastMessage
                    << " ParticipantController->Now()=" << now
                    << " Message contents=" << msg
                    << " delta=" << delta
                );
                throw std::runtime_error{"ValidationOnReceive: checking message contents failed delta="
                    + std::to_string(delta.count())};
            }
        }
        _validation.lastMessage = msg;
        if (_validation.lastTime > Nanoseconds::min())
        {
            auto delta = msg  - _validation.lastTime;
            // _validation.lastTime is the last step. msg might be the current or the next one
            // as some remote participant might have finished its simulation task already.
            if (delta > expectedTimeStep)
            {
                PRINT("    XXXXXXX " << _participantName << " UNEXPECTED MSG TIME: "
                    << " previous Now()=" << _validation.lastTime
                    << " ParticipantController->Now()=" << now
                    << " Message contents=" << msg
                    << " delta=" << delta
                );
                throw std::runtime_error{"ValidateOnReceive: checking timestamps failed delta=" + std::to_string(delta.count())};
            }
        }
        _validation.lastTime = now;
    }
    void OnParticipantStatus(sync::ParticipantStatus status) 
    {
        PRINT("    state " +_participantName + ":  "+ status.participantName + "=" + to_string(status.state));
    }

private:
    using Nanoseconds = std::chrono::nanoseconds;
    struct Validation
    {
        //!< we send the simulation time encoded in generic messages
        Nanoseconds lastMessage{ Nanoseconds::min()};
        //!< time stamp seen during asynchronous receive
        Nanoseconds lastTime{ Nanoseconds::min()};
        //!< time stamp seen in the simTask
        Nanoseconds lastSimTaskTime{ Nanoseconds::min()};

    };
    Validation _validation;
    ib::cfg::Config _config;
    ib::cfg::Participant _participantConfig;
    std::unique_ptr<IComAdapter> _comAdapter;
    std::string _participantName;
    std::future<ib::mw::sync::ParticipantState> _finalState;
    std::promise<void> _paused;
    bool _pauseInSimtask = false; //!< trigger test of Pause() from within simtask

    std::mutex _simTaskCvMutex;
    std::condition_variable _simTaskCv;
    bool _simTaskHasRun;
};

std::vector<std::unique_ptr<Participant>> participants;
size_t numberPauseSent{ 0 };
void pauseThread()
{
    isStarted.get_future().wait();
    while (!done)
    {
        for (auto&& participant : participants)
        {
            std::this_thread::sleep_for(50ms);
            if (done) break;

            if (participant->Name() == "S1")
            {
                PRINT("-!- " << "Triggering Pause in SimTask of Participant " << participant->Name());
                participant->PauseInSimtask();
            }
            else
            {
                PRINT("-!- " << "Sending Pause on Participant " << participant->Name());
                participant->Pause("External thread wants to pause " + participant->Name() + " for 5s");
            }
            numberPauseSent++;
            auto delay = 100ms + randomDelay();
            std::this_thread::sleep_for(delay);
            PRINT("-!- " << "Sending Continue on Participant " << participant->Name()
                << " delay was " << delay.count() << "ms");
            participant->Continue();
        }
    }
}

void controlThread()
{
    auto iter = std::find_if(participants.begin(), participants.end(), [](const std::unique_ptr<Participant>& p) -> bool { return p->Name() == "P1"; });
    if (iter != participants.end())
    {
      auto& participant = *iter;
      while (!done)
      {

        PRINT("-!- " << "Wait for SimTask ... " << participant->Name());
        participant->WaitForSimTask();
        PRINT("-!- " << "SimTask returned, sleeping ... " << participant->Name());
        std::this_thread::sleep_for(200ms);
        PRINT("-!- " << "completing SimTask ... " << participant->Name());
        participant->CompleteSimTask();
      }
    }
}

auto from_string(const std::string& value) -> ib::cfg::Middleware
{
    std::string lower;
    lower.resize(value.size());

    std::transform(value.begin(), value.end(), lower.begin(), [](auto ch) { return std::tolower(ch); });
    if (lower == "vasio")
    {
        return ib::cfg::Middleware::VAsio;
    }
    else if (lower == "fastrtps")
    {
        return ib::cfg::Middleware::FastRTPS;
    }
    return ib::cfg::Middleware::NotConfigured;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/
int main(int argc, char** argv)
{
    std::chrono::seconds timeout{0};
    // parse arguments for middleware config
    auto usage = []() {
        std::cout << "usage: IbDemoSimulationControl [--middleware VAsio|FastRTPS] [--timeout N]" << std::endl;
        std::cout << "   --middleware MW      Use the given middleware (VAsio, FastRTPS)" << std::endl;
        std::cout << "   --timeout    N       Terminate after N seconds." << std::endl; 
    };
    auto middleware = ib::cfg::Middleware::VAsio;
    auto getArg = [argc,argv](auto idx) -> const char* {
        if (idx + 1 == argc)
        {
            return nullptr;
        }
        return argv[idx + 1];
    };

    for (auto i = 1; i < argc; i++)
    {
        const auto arg = std::string{argv[i]};
        if (arg == "--middleware")
        {
            auto* argParameter = getArg(i);
            if (!argParameter)
            {
                usage();
                std::cout << "Error: `--middleware` expects an argument of FastRTPS or VAsio" << std::endl;
                return 1;
            }
            middleware = from_string(argParameter);
            if (middleware == ib::cfg::Middleware::NotConfigured)
            {
                usage();
                std::cout << "Error: Invalid middleware: " << argParameter << std::endl;
                return 1;
            }
            i++;
            continue;
        }
        else if (arg == "--timeout")
        {
            auto* argParameter = getArg(i);
            if (!argParameter)
            {
                usage();
                std::cout << "Error: `--timeout` expects the number of seconds as parameter"
                    << std::endl;
                return 1;
            }
            timeout = std::chrono::seconds{std::stol(argParameter)};
            i++;
            continue;
        }
        else if (arg == "-h" || arg == "--help")
        {
            usage();
            return 1;
        }
        else
        {
            usage();
            std::cout << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    //Dynamically create a simulation setup with some participants
    auto&& builder = ib::cfg::ConfigBuilder("SimulationControlDemo");
    builder.WithActiveMiddleware(middleware);

    auto&& simulationSetup = builder.SimulationSetup();
    simulationSetup.ConfigureTimeSync().WithTickPeriod(simulationPeriod);

    auto configurePublisher = [&simulationSetup](auto name, auto topic) {
        auto&& p = simulationSetup.AddParticipant(name);
        p.AddGenericPublisher(topic);
        p.AddParticipantController().WithSyncType(ib::cfg::SyncType::DistributedTimeQuantum);
    };
    auto configureSubscriber = [&simulationSetup](auto name, auto topic) {
        auto&& p = simulationSetup.AddParticipant(name);
        p.AddGenericSubscriber(topic);
        p.AddParticipantController().WithSyncType(ib::cfg::SyncType::DistributedTimeQuantum);
    };

    configurePublisher("P1", "Topic1");
    configureSubscriber("S1", "Topic1");
    configureSubscriber("S2", "Topic1");
    configureSubscriber("T1", "Topic1");
    configureSubscriber("T2", "Topic1");

    const auto config = builder.Build();

    // Start Registry service if necessary
    std::unique_ptr<ib::extensions::IIbRegistry> registry;
    if (config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::VAsio)
    {
        registry = ib::extensions::CreateIbRegistry(config);
        registry->ProvideDomain(g_domainId);
    }

    // Create the actual participant instances of the simulation
    for (const auto& participant : config.simulationSetup.participants)
    {
        participants.emplace_back(std::make_unique<Participant>(config, participant));
    }

    // send continue/pause signal from an 'external' thread
    auto sendPause = std::thread{ &pauseThread };
    auto control = std::thread{ &controlThread };

    if (timeout.count() > 0)
    {
        std::cout << "Running simulation. Exiting after timeout "
            << timeout.count() << "s"
            << std::endl;
        std::this_thread::sleep_for(timeout);
    }
    else
    {
        std::cout << "Running simulation. Press enter to exit." << std::endl;
        std::cin.ignore();
    }
    //clean up
    done = true;
    PRINT("Waiting for worker thread to stop...");
    if (sendPause.joinable())
    {
        sendPause.join();
    }
    
    if (control.joinable())
    {
        control.join();
    }
    

    return 0;
}
