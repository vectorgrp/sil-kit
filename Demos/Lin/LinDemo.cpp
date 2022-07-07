// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/string_utils.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/util/functional.hpp"

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
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_16(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_17(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_18(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_19(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_34(now); }},
            {5ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); }}
        };
    }

    void DoAction(std::chrono::nanoseconds now)
    {
        if (controller->Status() != LinControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrame_16(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }
        
    void SendFrame_17(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};

        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_18(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_19(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_34(std::chrono::nanoseconds /*now*/)
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 6;

        controller->SendFrame(frame, LinFrameResponseType::SlaveResponse);
        std::cout << "<< LIN Frame Header sent for ID=" << static_cast<unsigned int>(frame.id) << std::endl;
    }

    void GoToSleep()
    {
        std::cout << "<< Sending Go-To-Sleep Command and entering sleep state" << std::endl;
        controller->GoToSleep();
    }


    void ReceiveFrameStatus(ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent)
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

    void FrameStatusHandler(ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent)
    {
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

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
};


/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv) try 
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [registryUri]" << std::endl
                  << "Use \"LinMaster\" or \"LinSlave\" as <ParticipantName>." << std::endl;
        return -1;
    }

    std::string participantConfigurationFilename(argv[1]);
    std::string participantName(argv[2]);

    auto registryUri = "silkit://localdomain:8500";
    if (argc >= 4)
    {
        registryUri = argv[3];
    }

    auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

    std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
    auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
    auto* lifecycleService = participant->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();
    auto* linController = participant->CreateLinController("LIN1");

    // Set a Stop and Shutdown Handler
    lifecycleService->SetStopHandler([]() {
        std::cout << "Stopping..." << std::endl;
    });
    lifecycleService->SetShutdownHandler([]() {
        std::cout << "Shutting down..." << std::endl;
    });
    timeSyncService->SetPeriod(1ms);

    LinMaster master{linController};
    LinSlave slave;

    if (participantName == "LinMaster")
    {
        lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
            std::cout << "Initializing " << participantName << std::endl;

            LinControllerConfig config;
            config.controllerMode = LinControllerMode::Master;
            config.baudRate = 20'000;
            linController->Init(config);
        });
        linController->AddFrameStatusHandler(Util::bind_method(&master, &LinMaster::ReceiveFrameStatus));
        linController->AddWakeupHandler(Util::bind_method(&master, &LinMaster::WakeupHandler));

        timeSyncService->SetSimulationTask(
            [&master](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                master.DoAction(now);
            });
    }
    else
    {
        lifecycleService->SetCommunicationReadyHandler([&participantName, linController]() {
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
        });

        linController->AddFrameStatusHandler(Util::bind_method(&slave, &LinSlave::FrameStatusHandler));
        linController->AddGoToSleepHandler(Util::bind_method(&slave, &LinSlave::GoToSleepHandler));
        linController->AddWakeupHandler(Util::bind_method(&slave, &LinSlave::WakeupHandler));

        timeSyncService->SetSimulationTask(
            [&slave](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
                slave.DoAction(now);

                std::this_thread::sleep_for(500ms);
            });
    }

    auto lifecycleFuture = lifecycleService->StartLifecycleWithSyncTime(timeSyncService, {true, true});
    auto finalState = lifecycleFuture.get();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

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
