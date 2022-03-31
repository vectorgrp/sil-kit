#include <memory>
#include <thread>
#include <string>
#include <chrono>

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "gtest/gtest.h"

namespace {

using namespace ib::test;
using namespace ib::cfg;
using namespace ib;
using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::lin;
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
    std::map<FrameStatus, std::vector<Frame>> receivedFrames;
};

struct LinNode
{
    LinNode(IComAdapter* comAdapter, ILinController* controller, const std::string& name)
        : controller{controller}
        , _name{name}
        , _comAdapter{comAdapter}
    {
    }

    void Stop() 
    { 
        _comAdapter->GetSystemController()->Stop(); 
    }

    ILinController* controller{nullptr};
    std::string _name;
    ControllerConfig _controllerConfig;
    TestResult _result;
    IComAdapter* _comAdapter{nullptr};
};

class LinMaster : public LinNode
{
public:
    LinMaster(IComAdapter* comAdapter, ILinController* controller)
        : LinNode(comAdapter, controller, "LinMaster")
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

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
    }
        
    void SendFrame_17(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 17;
        frame.checksumModel = ChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1,7,1,7,1,7,1,7};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
    }

    void SendFrame_18(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 18;
        frame.checksumModel = ChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
    }

    void SendFrame_19(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 19;
        frame.checksumModel = ChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, FrameResponseType::MasterResponse, now);
    }

    void SendFrame_34(std::chrono::nanoseconds now)
    {
        Frame frame;
        frame.id = 34;
        frame.checksumModel = ChecksumModel::Enhanced;
        frame.dataLength = 6;

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, FrameResponseType::SlaveResponse, now);
    }

    void GoToSleep()
    {
        controller->GoToSleep();
        _result.gotoSleepSent = true;
    }

    void ReceiveFrameStatus(ILinController* /*controller*/, const Frame& frame, FrameStatus frameStatus, std::chrono::nanoseconds timestamp)
    {
        _result.receivedFrames[frameStatus].push_back(frame);
        _result.receiveTimes.push_back(timestamp);
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController)
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
    LinSlave(IComAdapter* comAdapter, ILinController* controller)
        : LinNode(comAdapter, controller, "LinSlave")
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* linController, const Frame& frame, FrameStatus status, std::chrono::nanoseconds /*timestamp*/)
    {
        _result.receivedFrames[status].push_back(frame);

        for (const auto& response: _controllerConfig.frameResponses)
        {
            if (linController->Status() == ControllerStatus::Sleep)
            {
              _result.numberReceivedInSleep++;
            }
            if (response.frame.id == frame.id && response.frame.checksumModel == frame.checksumModel)
            {
                _result.numberReceived++;
                if (_result.numberReceived == _controllerConfig.frameResponses.size())
                {
                  //Test finished
                }
            }
        }
    }

    void GoToSleepHandler(ILinController* linController)
    {
        // wakeup in 10 ms
        timer.Set(now + 10ms,
            [linController](std::chrono::nanoseconds /*now*/) {
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
    ControllerConfig config;
    config.controllerMode = ControllerMode::Master;
    config.baudRate = 20'000;

    if (participantName == "LinSlave")
    {
        config.controllerMode = ControllerMode::Slave;
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
        response_34.frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
        response_34.responseMode = FrameResponseMode::TxUnconditional;

        config.frameResponses.push_back(response_16);
        config.frameResponses.push_back(response_17);
        config.frameResponses.push_back(response_18);
        config.frameResponses.push_back(response_19);
        config.frameResponses.push_back(response_34);
    }
    return config;
}

class LinITest : public testing::Test
{
protected:
    LinITest() {}

    std::unique_ptr<SimTestHarness> _simTestHarness;
};

TEST_F(LinITest, sync_lin_simulation)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());
    std::vector<std::string> participantNames = { "LinMaster", "LinSlave" };
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, domainId, false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants
    {
        const std::string participantName = "LinMaster";
        auto&& comAdapter = _simTestHarness->GetParticipant(participantName)->ComAdapter();
        auto&& participantController = comAdapter->GetParticipantController();
        auto&& linController = comAdapter->CreateLinController("LinController1", "LIN_1");
        participantController->SetInitHandler([participantName, linController](auto /*initCmd*/) {
            auto config = MakeControllerConfig(participantName);
            linController->Init(config);
            });

        auto master = std::make_unique<LinMaster>(comAdapter, linController);

        linController->RegisterFrameStatusHandler(util::bind_method(master.get(), &LinMaster::ReceiveFrameStatus));
        linController->RegisterWakeupHandler(util::bind_method(master.get(), &LinMaster::WakeupHandler));

        participantController->SetSimulationTask(
            [master = master.get(), participantName](auto now) {
                master->doAction(now);
            });
        linNodes.emplace_back(std::move(master));
    }

    {
        const std::string participantName = "LinSlave";
        auto&& comAdapter = _simTestHarness->GetParticipant(participantName)->ComAdapter();
        auto&& participantController = comAdapter->GetParticipantController();
        auto&& linController = comAdapter->CreateLinController("LinController1", "LIN_1");


        auto config = MakeControllerConfig(participantName);
        participantController->SetInitHandler([config, participantName, linController](auto /*initCmd*/) {
            linController->Init(config);
          });

        auto slave = std::make_unique<LinSlave>(comAdapter, linController);
        linController->RegisterFrameStatusHandler(util::bind_method(slave.get(), &LinSlave::FrameStatusHandler));
        linController->RegisterGoToSleepHandler(util::bind_method(slave.get(), &LinSlave::GoToSleepHandler));

        //to validate the inputs
        slave->_controllerConfig = config;

        participantController->SetSimulationTask(
            [slave = slave.get()](auto now) {
                slave->DoAction(now);
            });
        linNodes.emplace_back(std::move(slave));
    }


    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // Sim is stopped when master received the wakeup pulse

    for (auto& node: linNodes)
    {
        if (node->_name == "LinSlave")
        {
            EXPECT_EQ(node->_result.numberReceivedInSleep, 0);
            EXPECT_TRUE(node->_result.gotoSleepReceived)
                << "Assuming node " << node->_name << " has received a GoToSleep";
            // The LinSlave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim),
            // so don't expect a wakeup
        }
        else
        {
            EXPECT_TRUE(node->_result.gotoSleepSent)
                << "Assuming node " << node->_name << " has received a GoToSleep";
            EXPECT_TRUE(node->_result.wakeupReceived) << "Assuming node " << node->_name << " has received a Wakeup";
        }
    }
    // Ensure that we are in a trivial simulation: the send and receive timestamps must be equal
    std::set<std::chrono::nanoseconds> merged;
    auto&& masterSendTimes = linNodes.at(0)->_result.sendTimes;
    auto&& masterRecvTimes = linNodes.at(0)->_result.receiveTimes;
    EXPECT_GT(masterSendTimes.size(), 0);
    EXPECT_GT(masterRecvTimes.size(), 0);
    EXPECT_EQ(masterSendTimes, masterRecvTimes)
      << "The master send times and receive times should be equal.";

    // The test runs for one schedule cycle with different messages/responses for master/slave
    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 4x acks with TX_OK for id 16,17,18,19 on master
    EXPECT_EQ(masterRecvFrames[FrameStatus::LIN_TX_OK].size(), 4); 
    // Only id 16 is valid for slave and received with LIN_RX_OK and given data
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    
    // id 17: sent with FrameResponseMode::Unused and should not trigger the reception callback for slaves
    // id 18: ChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    // id 19: dataLength does not match with master --> Receive with LIN_RX_ERROR
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_RX_ERROR].size(), 2);

    // id 34: sent by slave (slave should see TX, master should see RX with given data)
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[FrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[FrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[FrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[FrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
}

} //end namespace
