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

#include <future>
#include <iostream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/string_utils.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

using namespace SilKit::Services::Orchestration;
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
    auto ExecuteAction(std::chrono::nanoseconds now) -> bool
    {
        if (!_isActive || (now < _timeOut))
        {
            return false;
        }

        auto action = std::move(_action);
        Clear();
        action(now);

        return true;
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

    void ExecuteTask(std::chrono::nanoseconds now)
    {
        _now = now;
        if (_timer.ExecuteAction(now))
        {
            ScheduleNextTask();
        }
    }

private:
    void ScheduleNextTask()
    {
        auto currentTask = _nextTask++;
        if (_nextTask == _schedule.end())
        {
            _nextTask = _schedule.begin();
        }

        _timer.Set(_now + currentTask->delay, currentTask->action);
    }

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
            {10ms, [this](std::chrono::nanoseconds now) { SendFrame_16(now); }},
            {20ms, [this](std::chrono::nanoseconds now) { SendFrame_17(now); }},
            {10ms, [this](std::chrono::nanoseconds now) { SendFrame_18(now); }},
            {10ms, [this](std::chrono::nanoseconds now) { SendFrame_19(now); }},
            {10ms, [this](std::chrono::nanoseconds now) { SendFrame_34(now); }},
            {10ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); }}
        };
    }

    void DoAction(std::chrono::nanoseconds now)
    {
        schedule.ExecuteTask(now);
    }

    void SendFrame_16(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_17(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_18(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_19(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_34(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 6;

        SendFrameIfOperational(frame, LinFrameResponseType::SlaveResponse);
    }

    void GoToSleep()
    {
        std::cout << "<< Sending Go-To-Sleep Command and entering sleep state" << std::endl;
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
    }

private:
    void SendFrameIfOperational(const LinFrame& linFrame, LinFrameResponseType responseType)
    {
        const auto linId{static_cast<unsigned>(linFrame.id)};

        if (controller->Status() != LinControllerStatus::Operational)
        {
            std::cout << "!! LIN Frame with ID=" << linId << " not sent, since the controller is not operational"
                      << std::endl;
            return;
        }

        controller->SendFrame(linFrame, responseType);

        if (responseType == LinFrameResponseType::SlaveResponse)
        {
            std::cout << "<< LIN Frame Header sent for ID=" << linId << std::endl;
        }
        else
        {
            std::cout << "<< LIN Frame sent with ID=" << linId << std::endl;
        }
    }

private:
    ILinController* controller{nullptr};
    Schedule schedule;
};



class LinSlave
{
public:
    LinSlave() {}

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void UpdateTxBuffer_LinId34(ILinController* linController)
    {
        LinFrame frame34;
        frame34.id = 34;
        frame34.checksumModel = LinChecksumModel::Enhanced;
        frame34.dataLength = 6;
        frame34.data = {static_cast<uint8_t>(rand() % 10), 0, 0, 0, 0, 0, 0, 0};
        linController->UpdateTxBuffer(frame34);
    }

    void FrameStatusHandler(ILinController* linController, const LinFrameStatusEvent& frameStatusEvent)
    {
        // On a TX acknowledge for ID 34, update the TxBuffer for the next transmission
        if (frameStatusEvent.frame.id == 34)
        {
            UpdateTxBuffer_LinId34(linController);
        }

        std::cout << ">> " << frameStatusEvent.frame
                  << " status=" << frameStatusEvent.status
                  << " timestamp=" << frameStatusEvent.timestamp
                  << std::endl;
    }

    void GoToSleepHandler(ILinController* linController, const LinGoToSleepEvent& /*goToSleepEvent*/)
    {
        std::cout << "LIN Slave received go-to-sleep command; entering sleep mode." << std::endl;
        // wakeup in 15 ms
        timer.Set(now + 15ms,
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
        if (wakeupEvent.direction == SilKit::Services::TransmitDirection::RX)
        {
            linController->WakeupInternal();
        }
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
};

void InitLinMaster(ILinController* linController, std::string participantName)
{
    std::cout << "Initializing " << participantName << std::endl;

    LinControllerConfig config;
    config.controllerMode = LinControllerMode::Master;
    config.baudRate = 20'000;
    linController->Init(config);
}

void InitLinSlave(ILinController* linController, std::string participantName)
{
    std::cout << "Initializing " << participantName << std::endl;

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 16
    LinFrameResponse response_16;
    response_16.frame.id = 16;
    response_16.frame.checksumModel = LinChecksumModel::Classic;
    response_16.frame.dataLength = 6;
    response_16.responseMode = LinFrameResponseMode::Rx;

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 17
    //  - This LinFrameResponseMode::Unused causes the controller to ignore
    //    this message and not trigger a callback. This is also the default.
    LinFrameResponse response_17;
    response_17.frame.id = 17;
    response_17.frame.checksumModel = LinChecksumModel::Classic;
    response_17.frame.dataLength = 6;
    response_17.responseMode = LinFrameResponseMode::Unused;

    // Configure LIN Controller to receive LIN ID 18
    //  - LinChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    LinFrameResponse response_18;
    response_18.frame.id = 18;
    response_18.frame.checksumModel = LinChecksumModel::Classic;
    response_18.frame.dataLength = 8;
    response_18.responseMode = LinFrameResponseMode::Rx;

    // Configure LIN Controller to receive LIN ID 19
    //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
    LinFrameResponse response_19;
    response_19.frame.id = 19;
    response_19.frame.checksumModel = LinChecksumModel::Enhanced;
    response_19.frame.dataLength = 1;
    response_19.responseMode = LinFrameResponseMode::Rx;

    // Configure LIN Controller to send a LinFrameResponse for LIN ID 34
    LinFrameResponse response_34;
    response_34.frame.id = 34;
    response_34.frame.checksumModel = LinChecksumModel::Enhanced;
    response_34.frame.dataLength = 6;
    response_34.frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
    response_34.responseMode = LinFrameResponseMode::TxUnconditional;

    LinControllerConfig config;
    config.controllerMode = LinControllerMode::Slave;
    config.baudRate = 20'000;
    config.frameResponses.push_back(response_16);
    config.frameResponses.push_back(response_17);
    config.frameResponses.push_back(response_18);
    config.frameResponses.push_back(response_19);
    config.frameResponses.push_back(response_34);

    linController->Init(config);
}


/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv) try 
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri] [--async]" << std::endl
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

    auto operationMode = (runSync ? OperationMode::Coordinated : OperationMode::Autonomous);
    auto* lifecycleService = participant->CreateLifecycleService({operationMode});
    auto* timeSyncService = (runSync ? lifecycleService->CreateTimeSyncService() : nullptr);
    auto* linController = participant->CreateLinController("LIN1", "LIN1");

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

    LinMaster master{linController};
    LinSlave slave;

    if (participantName == "LinMaster")
    {
        linController->AddFrameStatusHandler(
            [&master](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
                master.FrameStatusHandler(linController, frameStatusEvent);
            });
        linController->AddWakeupHandler([&master](ILinController* linController, const LinWakeupEvent& wakeupEvent) {
                master.WakeupHandler(linController, wakeupEvent);
            });

        if (runSync)
        {
            lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
                InitLinMaster(linController, participantName);
            });
            timeSyncService->SetSimulationStepHandler(
                [&master](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                    master.DoAction(now);

                    std::this_thread::sleep_for(100ms);
                },
                1ms);

            auto lifecycleFuture = lifecycleService->StartLifecycle();
            auto finalState = lifecycleFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to end the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            std::atomic<bool> isStopRequested = {false};
            std::thread workerThread;
            auto now = 0ms;

            std::promise<void> startHandlerPromise;
            auto startHandlerFuture = startHandlerPromise.get_future();

            lifecycleService->SetCommunicationReadyHandler([&]() {
                InitLinMaster(linController, participantName);
                workerThread = std::thread{[&]() {
                    startHandlerFuture.get();
                    while (lifecycleService->State() == ParticipantState::ReadyToRun ||
                           lifecycleService->State() == ParticipantState::Running)
                    {
                        master.DoAction(now);
                        now += 1ms;
                        std::this_thread::sleep_for(200ms);
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
            std::cout << "Press enter to leave the simulation..." << std::endl;
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
    else if (participantName == "LinSlave")
    {
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

        if (runSync)
        {
            lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
                InitLinSlave(linController, participantName);
            });
            timeSyncService->SetSimulationStepHandler(
                [&slave](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms"
                              << std::endl;
                    slave.DoAction(now);

                    std::this_thread::sleep_for(100ms);
                },
                1ms);

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
            auto now = 0ms;
            std::promise<void> startHandlerPromise;
            auto startHandlerFuture = startHandlerPromise.get_future();

            lifecycleService->SetCommunicationReadyHandler([&]() {
                InitLinSlave(linController, participantName);
                workerThread = std::thread{[&]() {
                    startHandlerFuture.get();
                    while (lifecycleService->State() == ParticipantState::ReadyToRun ||
                           lifecycleService->State() == ParticipantState::Running)
                    {
                        slave.DoAction(now);
                        now += 1ms;
                        std::this_thread::sleep_for(200ms);
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
            std::cout << "Press enter to leave the simulation..." << std::endl;
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
