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
#include <memory>
#include <thread>
#include <string>
#include <chrono>

#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"

#include "gtest/gtest.h"

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit;
using namespace SilKit::Services;
using namespace SilKit::Services::Lin;

struct ITest_LinDemo : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

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

struct TestResult
{
    bool wakeupReceived{false};
    bool gotoSleepReceived{false};
    bool gotoSleepSent{false};
    size_t numberReceived{0}; //!< Number of received frames in slave
    size_t numberReceivedInSleep{0}; //!< Number of received frames while in sleepMode
    std::vector<std::chrono::nanoseconds> sendTimes;
    std::vector<std::chrono::nanoseconds> receiveTimes;
};

struct LinNode
{
    LinNode(IParticipant* participant, ILinController* controller, const std::string& name,
            Orchestration::ILifecycleService* lifecycleService)
        : _controller{controller}
        , _participant{participant}
        , _name{name}
        , _lifecycleService{lifecycleService}
    {
    }

    virtual ~LinNode() = default;

    void Stop() { _lifecycleService->Stop("Stop"); }

    ILinController* _controller{nullptr};
    IParticipant* _participant{nullptr};
    LinControllerConfig _controllerConfig;
    std::string _name;
    TestResult _result;
    Orchestration::ILifecycleService* _lifecycleService{nullptr};
};

class LinMaster : public LinNode
{
public:
    LinMaster(IParticipant* participant, ILinController* controller,
              Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinMaster", lifecycleService)
    {
        schedule = {
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_16(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_17(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_18(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_19(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_34(now); }},
            {5ms, [this](std::chrono::nanoseconds now) { GoToSleep(now); }}
        };
    }

    void doAction(std::chrono::nanoseconds now)
    {
        if (_controller->Status() != LinControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrame_16(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = Lin::LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        _result.sendTimes.push_back(now);
        _controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        Log() << "<< LIN LinFrame sent with ID=" << static_cast<uint16_t>(frame.id) << " now=" << now;
    }
        
    void SendFrame_17(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = Lin::LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};

        _result.sendTimes.push_back(now);
        _controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        Log() << "<< LIN LinFrame sent with ID=" << static_cast<uint16_t>(frame.id) << " now=" << now;
    }

    void SendFrame_18(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = Lin::LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        _controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        Log() << "<< LIN LinFrame sent with ID=" << static_cast<uint16_t>(frame.id) << " now=" << now;
    }

    void SendFrame_19(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = Lin::LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        _controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
        Log() << "<< LIN LinFrame sent with ID=" << static_cast<uint16_t>(frame.id) << " now=" << now;
    }

    void SendFrame_34(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = Lin::LinChecksumModel::Enhanced;
        frame.dataLength = 6;

        _result.sendTimes.push_back(now);
        _controller->SendFrame(frame, Lin::LinFrameResponseType::SlaveResponse);
        Log() << "<< LIN LinFrame Header sent for ID=" << static_cast<unsigned int>(frame.id) << " now=" << now;
    }

    void GoToSleep(std::chrono::nanoseconds now)
    {
        Log() << "<< Sending Go-To-Sleep Command and entering sleep state";
        _controller->GoToSleep();
        _result.gotoSleepSent = true;
        _result.sendTimes.push_back(now);

    }


    void ReceiveFrameStatus(ILinController* , const LinFrameStatusEvent& frameStatusEvent)
    {
        switch (frameStatusEvent.status)
        {
        case LinFrameStatus::LIN_RX_OK: break; // good case, no need to warn
        case LinFrameStatus::LIN_TX_OK: break; // good case, no need to warn
        default:
            Log() << "WARNING: LIN transmission failed!";
        }

        _result.receiveTimes.push_back(frameStatusEvent.timestamp);
        Log() << ">> " << frameStatusEvent.frame << " status=" << frameStatusEvent.status << " timestamp=" << frameStatusEvent.timestamp;
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* controller, const LinWakeupEvent& /*wakeupEvent*/)
    {
        if (controller->Status() != Lin::LinControllerStatus::Sleep)
            Log() << "WARNING: Received Wakeup pulse while ControllerStatus is " << controller->Status() << ".";

        Log() << ">> Wakeup pulse received";
        controller->WakeupInternal();
        schedule.ScheduleNextTask();
        _result.wakeupReceived = true;
    }

private:
    Schedule schedule;
};



class LinSlave : public LinNode
{
public:
    LinSlave(IParticipant* participant, ILinController* controller,
             Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinSlave", lifecycleService)
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* controller, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << frameStatusEvent.frame
                  << " status=" << frameStatusEvent.status
                  << " timestamp=" << frameStatusEvent.timestamp
                  ;
        for (const auto& response: _controllerConfig.frameResponses)
        {
            if (controller->Status() == LinControllerStatus::Sleep)
            {
              _result.numberReceivedInSleep++;
            }
            if (response.frame.id == frameStatusEvent.frame.id && response.frame.checksumModel == frameStatusEvent.frame.checksumModel)
            {
                _result.numberReceived++;
                if (_result.numberReceived == _controllerConfig.frameResponses.size())
                {
                  //Test finished
                    Stop();
                }
            }
        }
    }

    void GoToSleepHandler(ILinController* controller, const LinGoToSleepEvent& )
    {
        Log() << "LIN Slave received go-to-sleep command; entering sleep mode." ;
        // wakeup in 10 ms
        timer.Set(now + 10ms,
            [controller](std::chrono::nanoseconds now) {
                Log() << "<< Wakeup pulse now=" << now ;
                controller->Wakeup();
            });
        controller->GoToSleepInternal();
        _result.gotoSleepReceived = true;
    }

    void WakeupHandler(ILinController* controller, const LinWakeupEvent& )
    {
        Log() << "LIN Slave received wakeup pulse; entering normal operation mode." ;
        controller->WakeupInternal();
        _result.wakeupReceived = true;
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
};

auto MakeControllerConfig(const std::string& participantName)
{
  LinControllerConfig config;
    config.controllerMode = Lin::LinControllerMode::Master;
    config.baudRate = 20'000;

    if (participantName == "LinSlave")
    {
        config.controllerMode = Lin::LinControllerMode::Slave;
        // Configure LIN Controller to receive a FrameResponse for LIN ID 16
        LinFrameResponse response_16;
        response_16.frame.id = 16;
        response_16.frame.checksumModel = Lin::LinChecksumModel::Classic;
        response_16.frame.dataLength = 6;
        response_16.responseMode = LinFrameResponseMode::Rx;

        // Configure LIN Controller to receive a FrameResponse for LIN ID 17
        //  - This FrameResponseMode::Unused causes the controller to ignore
        //    this message and not trigger a callback. This is also the default.
        LinFrameResponse response_17;
        response_17.frame.id = 17;
        response_17.frame.checksumModel = Lin::LinChecksumModel::Classic;
        response_17.frame.dataLength = 6;
        response_17.responseMode = LinFrameResponseMode::Unused;

        // Configure LIN Controller to receive LIN ID 18
        //  - ChecksumModel does not match with master --> Receive with LIN_RX_ERROR
        LinFrameResponse response_18;
        response_18.frame.id = 18;
        response_18.frame.checksumModel = Lin::LinChecksumModel::Classic;
        response_18.frame.dataLength = 8;
        response_18.responseMode = LinFrameResponseMode::Rx;

        // Configure LIN Controller to receive LIN ID 19
        //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
        LinFrameResponse response_19;
        response_19.frame.id = 19;
        response_19.frame.checksumModel = Lin::LinChecksumModel::Enhanced;
        response_19.frame.dataLength = 1;
        response_19.responseMode = LinFrameResponseMode::Rx;

        // Configure LIN Controller to send a FrameResponse for LIN ID 34
        LinFrameResponse response_34;
        response_34.frame.id = 34;
        response_34.frame.checksumModel = Lin::LinChecksumModel::Enhanced;
        response_34.frame.dataLength = 6;
        response_34.frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
        response_34.responseMode = LinFrameResponseMode::TxUnconditional;

        config.frameResponses.push_back(response_16);
        config.frameResponses.push_back(response_17);
        config.frameResponses.push_back(response_18);
        config.frameResponses.push_back(response_19);
        config.frameResponses.push_back(response_34);
    }
    return config;
}

TEST_F(ITest_LinDemo, DISABLED_lin_demo)
{
    // Create required setup
    SetupFromParticipantList({"LinMaster", "LinSlave"});

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants and the netsim
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService =
            _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN1");
        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {

            Log() << "Initializing " << participantName;

            auto config = MakeControllerConfig(participantName);
            linController->Init(config);

            });

        auto master = std::make_unique<LinMaster>(participant, linController, lifecycleService);

        linController->AddFrameStatusHandler(
            [master = master.get()](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
                master->ReceiveFrameStatus(linController, frameStatusEvent);
            });
        linController->AddWakeupHandler(
            [master = master.get()](ILinController* linController, const LinWakeupEvent& wakeupEvent) {
                master->WakeupHandler(linController, wakeupEvent);
            });

        timeSyncService->SetSimulationStepHandler(
            [master = master.get(), participantName](auto now, std::chrono::nanoseconds /*duration*/) {

                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                Log() << participantName << " now=" << nowMs.count() << "ms";

                master->doAction(now);

            }, 1ms);
        linNodes.emplace_back(std::move(master));
    }

    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService =
            _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN1");


        auto config = MakeControllerConfig(participantName);
        lifecycleService->SetCommunicationReadyHandler([config, participantName, linController]() {

            Log() << " Initializing " << participantName;

            linController->Init(config);

          });

        auto slave = std::make_unique<LinSlave>(participant, linController, lifecycleService);

        linController->AddFrameStatusHandler(
            [slave = slave.get()](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
                slave->FrameStatusHandler(linController, frameStatusEvent);
            });
        linController->AddGoToSleepHandler(
            [slave = slave.get()](ILinController* linController, const LinGoToSleepEvent& goToSleepEvent) {
                slave->GoToSleepHandler(linController, goToSleepEvent);
            });
        linController->AddWakeupHandler(
            [slave = slave.get()](ILinController* linController, const LinWakeupEvent& wakeupEvent) {
                slave->WakeupHandler(linController, wakeupEvent);
            });

        //to validate the inputs
        slave->_controllerConfig = config;

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, std::chrono::nanoseconds /*duration*/) {

                Log() << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms";
                slave->DoAction(now);
            }, 1ms);
        linNodes.emplace_back(std::move(slave));
    }


    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    for (auto& node: linNodes)
    {
        if (node->_name == "LinSlave")
        {
            EXPECT_EQ(node->_result.numberReceivedInSleep, 0);
            EXPECT_EQ(node->_result.numberReceived, node->_controllerConfig.frameResponses.size());
            EXPECT_TRUE(node->_result.gotoSleepReceived)
                << "Assuming node " << node->_name << " has received a GoToSleep";
        }
        else
        {
            EXPECT_TRUE(node->_result.gotoSleepSent)
                << "Assuming node " << node->_name << " has received a GoToSleep";
        }
        EXPECT_TRUE(node->_result.wakeupReceived) << "Assuming node " << node->_name << " has received a Wakeup";
    }
    //Ensure that we are in a detailed simulation: the send and receive timestamps must not be equal
    std::set<std::chrono::nanoseconds> merged;
    auto&& masterSendTimes = linNodes.at(0)->_result.sendTimes;
    auto&& masterRecvTimes = linNodes.at(0)->_result.receiveTimes;

    EXPECT_GT(masterSendTimes.size(), 0u);
    EXPECT_GT(masterRecvTimes.size(), 0u);

    ASSERT_EQ(masterSendTimes.size(), masterRecvTimes.size());

    for (auto i = 0u; i < masterRecvTimes.size(); i++)
    {
        const auto& sendT = masterSendTimes.at(i);
        const auto& recvT = masterRecvTimes.at(i);
        const auto deltaT = recvT - sendT;
        EXPECT_TRUE(deltaT >= 0ms) << "received response before sending request (sendTime=" << sendT.count()
                                   << "ns, recvTime=" << recvT.count() << "ns)";
        EXPECT_TRUE(deltaT <= 1ms) << "received response more than one timestep after sending the request (sendTime="
                                   << sendT.count() << "ns, recvTime=" << recvT.count() << "ns)";
    }

    // Ensure that the receive times have no least significant digits (i.e., are rounded to 1ms)
    for (auto ts: masterRecvTimes)
    {
      using namespace std::chrono;
      //Round nanoseconds to milliseconds
      auto tMs = duration_cast<milliseconds>(ts);
      const auto diffTs = ts - duration_cast<nanoseconds>(tMs);
      EXPECT_EQ(diffTs.count(), (0ns).count())
        << "The simulated timestamps should have millisecond resolution in trivial simulation.";
    }
}

} //end namespace
