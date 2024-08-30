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

#include "SimTestHarness.hpp"

#include "gtest/gtest.h"

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit;
using namespace SilKit::Services;
using namespace SilKit::Services::Lin;
using namespace std::chrono_literals;

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
    Schedule(
        std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>> tasks)
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

    void SetCyclic(bool enabled)
    {
        _isCyclic = enabled;
    }

    void ScheduleNextTask()
    {
        if (_nextTask == _schedule.end())
        {
            return;
        }
        auto currentTask = _nextTask++;
        if (_isCyclic && (_nextTask == _schedule.end()))
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
    struct Task
    {
        Task(std::chrono::nanoseconds delay, std::function<void(std::chrono::nanoseconds)> action)
            : delay{delay}
            , action{action}
        {
        }

        std::chrono::nanoseconds delay;
        std::function<void(std::chrono::nanoseconds)> action;
    };

    Timer _timer;
    std::vector<Task> _schedule;
    std::vector<Task>::iterator _nextTask;
    std::chrono::nanoseconds _now = 0ns;
    bool _isCyclic{false};
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
    std::map<LinFrameStatus, std::vector<LinFrame>> receivedFrames;
};

struct LinNode
{
    LinNode(IParticipant* participant, ILinController* controller, const std::string& name,
            Orchestration::ILifecycleService* lifecycleService)
        : controller{controller}
        , _name{name}
        , _participant{participant}
        , _lifecycleService{lifecycleService}
    {
    }

    virtual ~LinNode() = default;

    void Stop()
    {
        _lifecycleService->Stop("Stop");
    }

    ILinController* controller{nullptr};
    std::string _name;
    TestResult _result;
    IParticipant* _participant{nullptr};
    Orchestration::ILifecycleService* _lifecycleService{nullptr};
};

class LinMaster : public LinNode
{
public:
    LinMaster(IParticipant* participant, ILinController* controller, Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinMaster", lifecycleService)
    {
        schedule = {
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_16(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_17(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_18(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_19(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrame_34(now); }},
            {5ms, [this](std::chrono::nanoseconds now) { GoToSleep(now); }},
        };
    }

    void doAction(std::chrono::nanoseconds now)
    {
        if (controller->Status() != LinControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrame_16(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_17(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_18(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_19(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_34(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 6;

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::SlaveResponse);
    }

    void GoToSleep(std::chrono::nanoseconds now)
    {
        controller->GoToSleep();
        _result.gotoSleepSent = true;
        _result.sendTimes.push_back(now);
    }

    void ReceiveFrameStatus(ILinController* /*controller*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        _result.receivedFrames[frameStatusEvent.status].push_back(frameStatusEvent.frame);
        _result.receiveTimes.push_back(frameStatusEvent.timestamp);
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController, const LinWakeupEvent& /*wakeupEvent*/)
    {
        linController->WakeupInternal();
        _result.wakeupReceived = true;
        // No further schedule, stop simulation after one cycle
        Stop();
    }

private:
    Schedule schedule;
};


class LinSlave : public LinNode
{
public:
    LinSlave(IParticipant* participant, ILinController* controller, Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinSlave", lifecycleService)
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* linController, const LinFrameStatusEvent& frameStatusEvent)
    {
        _result.receivedFrames[frameStatusEvent.status].push_back(frameStatusEvent.frame);

        if (linController->Status() == LinControllerStatus::Sleep)
        {
            _result.numberReceivedInSleep++;
        }
        else
        {
            _result.numberReceived++;
        }
    }

    void GoToSleepHandler(ILinController* linController, const LinGoToSleepEvent& /*goToSleepEvent*/)
    {
        // wakeup in 10 ms
        timer.Set(now + 10ms, [linController](std::chrono::nanoseconds /*now*/) {
            linController->Wakeup();
            // The LinSlave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim)
        });
        linController->GoToSleepInternal();
        _result.gotoSleepReceived = true;
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
};

auto MakeControllerConfig(const std::string& participantName)
{
    LinControllerConfig config;
    config.controllerMode = LinControllerMode::Master;
    config.baudRate = 20'000;

    if (participantName == "LinSlave")
    {
        config.controllerMode = LinControllerMode::Slave;
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

        LinFrameResponse response_60;
        response_60.frame = GoToSleepFrame();
        response_60.responseMode = LinFrameResponseMode::Rx;

        config.frameResponses.push_back(response_16);
        config.frameResponses.push_back(response_17);
        config.frameResponses.push_back(response_18);
        config.frameResponses.push_back(response_19);
        config.frameResponses.push_back(response_34);
        config.frameResponses.push_back(response_60);
    }
    return config;
}

class ITest_Lin : public testing::Test
{
protected:
    ITest_Lin() {}

    std::unique_ptr<SimTestHarness> _simTestHarness;
};

TEST_F(ITest_Lin, sync_lin_simulation)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");
        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
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
            master->doAction(now);
        }, 1ms);
        linNodes.emplace_back(std::move(master));
    }

    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");


        auto config = MakeControllerConfig(participantName);
        lifecycleService->SetCommunicationReadyHandler(
            [config, participantName, linController]() { linController->Init(config); });

        auto slave = std::make_unique<LinSlave>(participant, linController, lifecycleService);

        linController->AddFrameStatusHandler(
            [slave = slave.get()](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });
        linController->AddGoToSleepHandler(
            [slave = slave.get()](ILinController* linController, const LinGoToSleepEvent& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, std::chrono::nanoseconds /*duration*/) { slave->DoAction(now); }, 1ms);
        linNodes.emplace_back(std::move(slave));
    }


    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // Sim is stopped when master received the wakeup pulse

    for (auto& node : linNodes)
    {
        if (node->_name == "LinSlave")
        {
            EXPECT_EQ(node->_result.numberReceivedInSleep, 0u);
            EXPECT_TRUE(node->_result.gotoSleepReceived)
                << "Assuming node " << node->_name << " has received a GoToSleep";
            // The LinSlave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim),
            // so don't expect a wakeup
        }
        else
        {
            EXPECT_TRUE(node->_result.gotoSleepSent) << "Assuming node " << node->_name << " has received a GoToSleep";
            EXPECT_TRUE(node->_result.wakeupReceived) << "Assuming node " << node->_name << " has received a Wakeup";
        }
    }
    // Ensure that we are in a trivial simulation: the send and receive timestamps must be equal
    std::set<std::chrono::nanoseconds> merged;
    auto&& masterSendTimes = linNodes.at(0)->_result.sendTimes;
    auto&& masterRecvTimes = linNodes.at(0)->_result.receiveTimes;
    EXPECT_GT(masterSendTimes.size(), 0u);
    EXPECT_GT(masterRecvTimes.size(), 0u);
    ASSERT_EQ(masterSendTimes.size(), masterRecvTimes.size())
        << "The master send times and receive times should have equal size.";
    for (auto i = 0u; i < masterRecvTimes.size(); i++)
    {
        const auto& sendT = masterSendTimes.at(i);
        const auto& recvT = masterRecvTimes.at(i);
        const auto deltaT = std::abs(sendT.count() - recvT.count());
        const auto deltaNs = std::chrono::nanoseconds{deltaT};
        EXPECT_TRUE(deltaNs == 1ms || deltaNs == 0ms) << "DeltaT is " << deltaT;
    }

    // The test runs for one schedule cycle with different messages/responses for master/slave

    // Id       Master          Slave
    // 16       LIN_TX_OK       RX_OK
    // 17       LIN_TX_OK       IGNORED!
    // 18       LIN_TX_OK       LIN_RX_ERROR due to checksum
    // 19       LIN_TX_OK       LIN_RX_ERROR due to datalength
    // 34       RX_OK           LIN_TX_OK

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 5x acks with TX_OK for id 16,17,18,19,60 on master
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 5);

    // LIN_RX_OK for Id 16 and GoToSleep-Frame
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 2);
    // Id 16 is valid for slave and received with LIN_RX_OK and given data
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    // GoToSleep-Frame with fixed id=60 and first byte to 0
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 60);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].data[0], 0);

    // id 17: sent with LinFrameResponseMode::Unused and should not trigger the reception callback for slaves
    // id 18: LinChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    // id 19: dataLength does not match with master --> Receive with LIN_RX_ERROR
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR].size(), 2);

    // id 34: sent by slave (slave should see TX, master should see RX with given data)
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
}

} //end namespace
