// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/functional.hpp"

using namespace ib;
using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::lin;

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

    void doAction(std::chrono::nanoseconds now)
    {
        if (controller->Status() != ControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrame_16(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 16;
        frame.checksumModel = ChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }
        
    void SendFrame_17(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 17;
        frame.checksumModel = ChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};

        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_18(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 18;
        frame.checksumModel = ChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_19(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 19;
        frame.checksumModel = ChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
        std::cout << "<< LIN Frame sent with ID=" << static_cast<uint16_t>(frame.id) << std::endl;
    }

    void SendFrame_34(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 34;
        frame.checksumModel = ChecksumModel::Enhanced;
        frame.dataLength = 6;

        controller->SendFrame(frame, FrameResponseType::SlaveResponse, now);
        std::cout << "<< LIN Frame Header sent for ID=" << static_cast<unsigned int>(frame.id) << std::endl;
    }

    void GoToSleep()
    {
        std::cout << "<< Sending Go-To-Sleep Command and entering sleep state" << std::endl;
        controller->GoToSleep();
    }


    void ReceiveFrameStatus(ILinController* /*linController*/, const Frame& frame, FrameStatus frameStatus, std::chrono::nanoseconds /*timestamp*/)
    {
        switch (frameStatus)
        {
        case FrameStatus::LIN_RX_OK: break; // good case, no need to warn
        case FrameStatus::LIN_TX_OK: break; // good case, no need to warn
        default:
            std::cout << "WARNING: LIN transmission failed!" << std::endl;
        }

        std::cout << ">> " << frame << " status=" << frameStatus << std::endl;
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController)
    {
        if (linController->Status() != ControllerStatus::Sleep)
            std::cout << "WARNING: Received Wakeup pulse while ControllerStatus is " << linController->Status() << "." << std::endl;

        std::cout << ">> Wakeup pulse received" << std::endl;
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

    void FrameStatusHandler(ILinController* /*linController*/, const Frame& frame, FrameStatus status, std::chrono::nanoseconds timestamp)
    {
        std::cout << ">> " << frame
                  << " status=" << status
                  << " timestamp=" << timestamp
                  << std::endl;
    }

    void GoToSleepHandler(ILinController* linController)
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

    void WakeupHandler(ILinController* linController)
    {
        std::cout << "LIN Slave received wakeup pulse; entering normal operation mode." << std::endl;
        linController->WakeupInternal();
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
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }

    std::string participantConfigurationFilename(argv[1]);
    std::string participantName(argv[2]);

    uint32_t domainId = 42;
    if (argc >= 4)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[3]));
    }

    auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto participant = ib::CreateSimulationParticipant(participantConfiguration, participantName, domainId, true);
    auto* participantController = participant->GetParticipantController();
    auto* linController = participant->CreateLinController("LIN1");

    // Set a Stop and Shutdown Handler
    participantController->SetStopHandler([]() { std::cout << "Stopping..." << std::endl; });
    participantController->SetShutdownHandler([]() { std::cout << "Shutting down..." << std::endl; });
    participantController->SetPeriod(1ms);

    LinMaster master{linController};
    LinSlave slave;

    if (participantName == "LinMaster")
    {
        participantController->SetInitHandler([&participantName, linController](auto /*initCmd*/) {

            std::cout << "Initializing " << participantName << std::endl;

            ControllerConfig config;
            config.controllerMode = ControllerMode::Master;
            config.baudRate = 20'000;
            linController->Init(config);

            });

        linController->RegisterFrameStatusHandler(util::bind_method(&master, &LinMaster::ReceiveFrameStatus));
        linController->RegisterWakeupHandler(util::bind_method(&master, &LinMaster::WakeupHandler));

        participantController->SetSimulationTask(
            [&master](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                master.doAction(now);

            });
    }
    else
    {
        participantController->SetInitHandler([&participantName, linController](auto /*initCmd*/) {

            std::cout << "Initializing " << participantName << std::endl;

            // Configure LIN Controller to receive a FrameResponse for LIN ID 16
            FrameResponse response_16;
            response_16.frame.id = 16;
            response_16.frame.checksumModel = ChecksumModel::Classic;
            response_16.frame.dataLength = 6;
            response_16.responseMode = FrameResponseMode::Rx;

            // Configure LIN Controller to receive a FrameResponse for LIN ID 17
            //  - This FrameResponseMode::Unused causes the controller to ignore
            //    this message and not trigger a callback. This is also the default.
            FrameResponse response_17;
            response_17.frame.id = 17;
            response_17.frame.checksumModel = ChecksumModel::Classic;
            response_17.frame.dataLength = 6;
            response_17.responseMode = FrameResponseMode::Unused;

            // Configure LIN Controller to receive LIN ID 18
            //  - ChecksumModel does not match with master --> Receive with LIN_RX_ERROR
            FrameResponse response_18;
            response_18.frame.id = 18;
            response_18.frame.checksumModel = ChecksumModel::Classic;
            response_18.frame.dataLength = 8;
            response_18.responseMode = FrameResponseMode::Rx;

            // Configure LIN Controller to receive LIN ID 19
            //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
            FrameResponse response_19;
            response_19.frame.id = 19;
            response_19.frame.checksumModel = ChecksumModel::Enhanced;
            response_19.frame.dataLength = 1;
            response_19.responseMode = FrameResponseMode::Rx;


            // Configure LIN Controller to send a FrameResponse for LIN ID 34
            FrameResponse response_34;
            response_34.frame.id = 34;
            response_34.frame.checksumModel = ChecksumModel::Enhanced;
            response_34.frame.dataLength = 6;
            response_34.frame.data = std::array<uint8_t, 8>{3,4,3,4,3,4,3,4};
            response_34.responseMode = FrameResponseMode::TxUnconditional;

            ControllerConfig config;
            config.controllerMode = ControllerMode::Slave;
            config.baudRate = 20'000;
            config.frameResponses.push_back(response_16);
            config.frameResponses.push_back(response_17);
            config.frameResponses.push_back(response_18);
            config.frameResponses.push_back(response_19);
            config.frameResponses.push_back(response_34);

            linController->Init(config);
            });

        linController->RegisterFrameStatusHandler(util::bind_method(&slave, &LinSlave::FrameStatusHandler));
        linController->RegisterGoToSleepHandler(util::bind_method(&slave, &LinSlave::GoToSleepHandler));
        linController->RegisterWakeupHandler(util::bind_method(&slave, &LinSlave::WakeupHandler));

        participantController->SetSimulationTask(
            [&slave](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
                slave.DoAction(now);

                std::this_thread::sleep_for(500ms);
            });
    }

    auto finalStateFuture = participantController->RunAsync();
    auto finalState = finalStateFuture.get();

    //auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}
catch (const ib::ConfigurationError& error)
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
