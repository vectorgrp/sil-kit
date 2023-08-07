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
#include <thread>
#include <unordered_map>

#include "silkit/SilKit.hpp"
#include "silkit/services/string_utils.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

using namespace SilKit;

using namespace SilKit::Services;
using namespace SilKit::Services::Lin;

using namespace std::chrono_literals;
using namespace std::placeholders;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timestamp);
    out << seconds.count() << "ms";
    return out;
}

class Timer
{
public:
    void Set(std::chrono::nanoseconds timeOut, std::function<void(std::chrono::nanoseconds)> action) noexcept
    {
        _isActive = true;
        _timeOut = timeOut;
        _action = std::move(action);
    }
    void Clear() noexcept
    {
        _isActive = false;
        _timeOut = std::chrono::nanoseconds::max();
        _action = std::function<void(std::chrono::nanoseconds)>{};
    }
    void ExecuteAction(std::chrono::nanoseconds now)
    {
        if (!_isActive || (now < _timeOut))
            return;

        auto action = std::move(_action);
        Clear();
        action(now);
    }

private:
    bool _isActive = false;
    std::chrono::nanoseconds _timeOut = std::chrono::nanoseconds::max();
    std::function<void(std::chrono::nanoseconds)> _action;
};

class Schedule
{
public:
    Schedule() = default;
    Schedule(std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>> tasks)
    {
        for (auto&& task : tasks)
        {
            _schedule.emplace_back(task.first, task.second);
        }
        Reset();
    }

    void Reset()
    {
        _nextTask = _schedule.begin();
        ScheduleNextTask();
    }

    void ScheduleNextTask()
    {
        auto currentTask = _nextTask++;
        if (_nextTask == _schedule.end())
        {
            _nextTask = _schedule.begin();
        }

        _timer.Set(_now + currentTask->delay, currentTask->action);
    }

    void ExecuteTask(std::chrono::nanoseconds now)
    {
        _now = now;
        _timer.ExecuteAction(now);
    }

private:
    struct Task {
        Task(std::chrono::nanoseconds delay, std::function<void(std::chrono::nanoseconds)> action) : delay{delay}, action{action} {}

        std::chrono::nanoseconds delay;
        std::function<void(std::chrono::nanoseconds)> action;
    };

    Timer _timer;
    std::vector<Task> _schedule;
    std::vector<Task>::iterator _nextTask;
    std::chrono::nanoseconds _now = 0ns;
};

class LinMaster
{
public:
    LinMaster(ILinController* controller)
        : controller{controller}
    {
        schedule = {
            {5ms, [this](std::chrono::nanoseconds now) { SendFrameHeader(now, 16); }},
            {5ms, [this](std::chrono::nanoseconds now) { SendFrameHeader(now, 17); }},
            {5ms, [this](std::chrono::nanoseconds now) { SendFrameHeader(now, 18); }},
            {5ms, [this](std::chrono::nanoseconds now) { SendFrameHeader(now, 19); }},
            {5ms, [this](std::chrono::nanoseconds now) { SendFrameHeader(now, 34); }},
            {5ms, [this](std::chrono::nanoseconds now) { GoToSleep(now); }},
        };

        LinFrame f16{};
        f16.id = 16;
        f16.checksumModel = LinChecksumModel::Classic;
        f16.dataLength = 6;
        f16.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};
        _masterResponses[f16.id] = f16;

        LinFrame f17{};
        f17.id = 17;
        f17.checksumModel = LinChecksumModel::Classic;
        f17.dataLength = 6;
        f17.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};
        _masterResponses[f17.id] = f17;

        LinFrame f18{};
        f18.id = 18;
        f18.checksumModel = LinChecksumModel::Enhanced;
        f18.dataLength = 8;
        f18.data = std::array<uint8_t, 8>{0};
        _masterResponses[f18.id] = f18;

        LinFrame f19{};
        f19.id = 19;
        f19.checksumModel = LinChecksumModel::Classic;
        f19.dataLength = 8;
        f19.data = std::array<uint8_t, 8>{0};
        _masterResponses[f19.id] = f19;
    }

    void DoAction(std::chrono::nanoseconds now)
    {
        if (controller->Status() != LinControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrameHeader(std::chrono::nanoseconds now, LinId linId)
    {
        controller->SendFrameHeader(linId);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(linId) << " @" << now.count() << "ns" << std::endl;
    }

    void GoToSleep(std::chrono::nanoseconds now)
    {
        std::cout << "<< Sending Go-To-Sleep Command and entering sleep state @" << now.count() << "ns" << std::endl;
        controller->GoToSleep();
    }

    void FrameStatusHandler(ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        switch (frameStatusEvent.status)
        {
        case LinFrameStatus::LIN_RX_OK: break; // good case, no need to warn
        case LinFrameStatus::LIN_TX_OK: break; // good case, no need to warn
        default:
            std::cout << "WARNING: LIN transmission failed!" << std::endl;
        }

        std::cout << ">> " << frameStatusEvent.frame << " status=" << frameStatusEvent.status << std::endl;
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController, const LinWakeupEvent& wakeupEvent)
    {
        if (linController->Status() != LinControllerStatus::Sleep)
        {
            std::cout << "WARNING: Received Wakeup pulse while LinControllerStatus is " << linController->Status()
                      << "." << std::endl;
        }

        std::cout << ">> Wakeup pulse received; direction=" << wakeupEvent.direction << std::endl;
        linController->WakeupInternal();
        schedule.ScheduleNextTask();
    }

    void OnFrameHeader(ILinController* linController, const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& header)
    {
        std::cout << ">> Received Frame Header: id=" << (int)header.id << "@" << header.timestamp << std::endl;

        const auto it = _masterResponses.find(header.id);
        if (it != _masterResponses.end())
        {
            const auto& frame = it->second;
            SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            std::cout << "<< Sending dynamic response: id=" << static_cast<int>(header.id) << std::endl;
        }
        else
        {
            std::cout << "!! Not sending dynamic response: id=" << static_cast<int>(header.id) << std::endl;
        }
    }

private:
    ILinController* controller{nullptr};
    Schedule schedule;
    std::unordered_map<LinId, LinFrame> _masterResponses;
};

class LinSlave
{
public:
    LinSlave()
    {
        UpdateDynamicResponseTo34();
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void UpdateDynamicResponseTo34()
    {
        LinFrame f34{};
        f34.id = 34;
        f34.checksumModel = LinChecksumModel::Enhanced;
        f34.dataLength = 6;
        f34.data = {static_cast<uint8_t>(rand() % 10), 0, 0, 0, 0, 0, 0, 0};
        _slaveResponses[f34.id] = f34;
    }

    void FrameStatusHandler(ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        // On a TX acknowledge for ID 34, update the TxBuffer for the next transmission
        if (frameStatusEvent.frame.id == 34)
        {
            UpdateDynamicResponseTo34();
        }

        std::cout << ">> " << frameStatusEvent.frame
                  << " status=" << frameStatusEvent.status
                  << " timestamp=" << frameStatusEvent.timestamp
                  << std::endl;
    }

    void GoToSleepHandler(ILinController* linController, const LinGoToSleepEvent& /*goToSleepEvent*/)
    {
        std::cout << "LIN Slave received go-to-sleep command; entering sleep mode." << std::endl;
        // wakeup in 10 ms
        timer.Set(now + 10ms,
            [linController](std::chrono::nanoseconds tnow) {
                std::cout << "<< Wakeup pulse @" << tnow << std::endl;
                linController->Wakeup();
            });
        linController->GoToSleepInternal();
    }

    void WakeupHandler(ILinController* linController, const LinWakeupEvent& wakeupEvent)
    {
        std::cout << "LIN Slave received wakeup pulse; direction=" << wakeupEvent.direction
                  << "; Entering normal operation mode." << std::endl;

        // No need to set the controller status if we sent the wakeup
        if (wakeupEvent.direction == TransmitDirection::RX)
        {
            linController->WakeupInternal();
        }
    }

    void OnFrameHeader(ILinController* linController, const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& header)
    {
        std::cout << "<< Received Frame Header: id=" << (int)header.id << "@" << header.timestamp << std::endl;

        const auto it = _slaveResponses.find(header.id);
        if (it != _slaveResponses.end())
        {
            const auto& frame = it->second;
            SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            std::cout << ">> Sending dynamic response: id=" << static_cast<int>(header.id) << std::endl;
        }
        else
        {
            std::cout << "!! Not sending dynamic response: id=" << static_cast<int>(header.id) << std::endl;
        }
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
    std::unordered_map<LinId, LinFrame> _slaveResponses;
};

void InitLinMaster(SilKit::Services::Lin::ILinController* linController, std::string participantName)
{
    std::cout << "Initializing " << participantName << std::endl;

    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config;
    config.controllerMode = LinControllerMode::Master;
    config.baudRate = 20'000;

    SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
}

void InitLinSlave(SilKit::Services::Lin::ILinController* linController, std::string participantName)
{
    std::cout << "Initializing " << participantName << std::endl;

    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config{};
    config.controllerMode = LinControllerMode::Slave;
    config.baudRate = 20'000;

    SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
}


/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv) try
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl
                  << "Use \"LinMaster\" or \"LinSlave\" as <ParticipantName>." << std::endl;
        return -1;
    }

    std::string participantConfigurationFilename(argv[1]);
    std::string participantName(argv[2]);

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
    auto* lifecycleService =
        participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();
    auto* linController = participant->CreateLinController("LIN1", "LIN1");

    // Set a Stop and Shutdown Handler
    lifecycleService->SetStopHandler([]() {
        std::cout << "Stop handler called" << std::endl;
    });
    lifecycleService->SetShutdownHandler([]() {
        std::cout << "Shutdown handler called" << std::endl;
    });

    LinMaster master{linController};
    LinSlave slave;

    if (participantName == "LinMaster")
    {

        lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
            InitLinMaster(linController, participantName);
        });
        linController->AddFrameStatusHandler(
            [&master](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
                master.FrameStatusHandler(linController, frameStatusEvent);
            });
        linController->AddWakeupHandler([&master](ILinController* linController, const LinWakeupEvent& wakeupEvent) {
                master.WakeupHandler(linController, wakeupEvent);
            });

        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController, [&master](ILinController* linController,
                                     const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& event) {
                master.OnFrameHeader(linController, event);
            });

        if (runSync)
        {
            timeSyncService->SetSimulationStepHandler(
                [&master](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                    master.DoAction(now);
                },
                1ms);


            auto lifecycleFuture = lifecycleService->StartLifecycle();
            auto finalState = lifecycleFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            InitLinMaster(linController, participantName);

            bool isStopped = false;
            std::thread workerThread;
            auto now = 0ms;

            workerThread = std::thread{[&]() {
                while (!isStopped)
                {
                    master.DoAction(now);
                    now += 1ms;
                    std::this_thread::sleep_for(200ms);
                }
            }};

            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
            isStopped = true;
            if (workerThread.joinable())
            {
                workerThread.join();
            }
        }
    }
    else if (participantName == "LinSlave")
    {
        lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
            InitLinSlave(linController, participantName);
        });

        linController->AddFrameStatusHandler(
            [&slave](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
                slave.FrameStatusHandler(linController, frameStatusEvent);
            });
        linController->AddGoToSleepHandler(
            [&slave](ILinController* linController, const LinGoToSleepEvent& goToSleepEvent) {
                slave.GoToSleepHandler(linController, goToSleepEvent);
            });
        linController->AddWakeupHandler([&slave](ILinController* linController, const LinWakeupEvent& wakeupEvent) {
                slave.WakeupHandler(linController, wakeupEvent);
            });

        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController, [&slave](ILinController* linController,
                const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& event) {
              slave.OnFrameHeader(linController, event);
            });

        if (runSync)
        {
            timeSyncService->SetSimulationStepHandler(
                [&slave](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms"
                              << std::endl;
                    slave.DoAction(now);

                    std::this_thread::sleep_for(100ms);
                },
                1ms);

            auto lifecycleFuture = lifecycleService->StartLifecycle();
            auto finalState = lifecycleFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;

            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            InitLinSlave(linController, participantName);

            bool isStopped = false;
            std::thread workerThread;
            auto now = 0ms;

            workerThread = std::thread{[&]() {
                while (!isStopped)
                {
                    slave.DoAction(now);
                    now += 1ms;
                    std::this_thread::sleep_for(200ms);
                }
            }};

            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
            isStopped = true;
            if (workerThread.joinable())
            {
                workerThread.join();
            }
        }
    }
    else
    {
        std::cout << "Wrong participant name provided. Use either \"LinMaster\" or \"LinSlave\"."
                  << std::endl;
        return 1;
    }


    return 0;
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
