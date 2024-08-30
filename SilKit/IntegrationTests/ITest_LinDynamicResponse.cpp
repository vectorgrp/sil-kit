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
#include <deque>
#include <unordered_map>

#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

#include "SimTestHarness.hpp"
#include "ITestThreadSafeLogger.hpp"

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

class LinDynamicMaster : public LinNode
{
public:
    LinDynamicMaster(IParticipant* participant, ILinController* controller,
                     Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinMaster", lifecycleService)
    {
        schedule = {
            {0ns, [this](std::chrono::nanoseconds now) { SendFrameHeader_16(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrameHeader_17(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrameHeader_18(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrameHeader_19(now); }},
            {0ns, [this](std::chrono::nanoseconds now) { SendFrameHeader_34(now); }},
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
        f17.data = std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7};
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

    void doAction(std::chrono::nanoseconds now)
    {
        if (controller->Status() != LinControllerStatus::Operational)
            return;

        schedule.ExecuteTask(now);
    }

    void SendFrameHeader_16(std::chrono::nanoseconds now)
    {
        _result.sendTimes.push_back(now);
        controller->SendFrameHeader(16);
    }

    void SendFrameHeader_17(std::chrono::nanoseconds now)
    {
        _result.sendTimes.push_back(now);
        controller->SendFrameHeader(17);
    }

    void SendFrameHeader_18(std::chrono::nanoseconds now)
    {
        _result.sendTimes.push_back(now);
        controller->SendFrameHeader(18);
    }

    void SendFrameHeader_19(std::chrono::nanoseconds now)
    {
        _result.sendTimes.push_back(now);
        controller->SendFrameHeader(19);
    }

    void SendFrameHeader_34(std::chrono::nanoseconds now)
    {
        _result.sendTimes.push_back(now);
        controller->SendFrameHeader(34);
    }

    void GoToSleep(std::chrono::nanoseconds now)
    {
        controller->GoToSleep();
        _result.gotoSleepSent = true;
        _result.sendTimes.push_back(now);
    }

    void ReceiveFrameStatus(ILinController* /*controller*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

        _result.receivedFrames[frameStatusEvent.status].push_back(frameStatusEvent.frame);
        _result.receiveTimes.push_back(frameStatusEvent.timestamp);
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController, const LinWakeupEvent& /*wakeupEvent*/)
    {
        Log() << ">> " << _name << " wakeup pulse received";

        linController->WakeupInternal();
        _result.wakeupReceived = true;
        // No further schedule, stop simulation after one cycle
        Stop();
    }

    void OnFrameHeader(ILinController* linController,
                       const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& header)
    {
        Log() << ">> " << _name << " received frame header: id=" << (int)header.id << "@" << header.timestamp;

        const auto it = _masterResponses.find(header.id);
        if (it != _masterResponses.end())
        {
            const auto& frame = it->second;
            SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            Log() << "<< " << _name << " sending dynamic response: id=" << static_cast<int>(header.id);
            ASSERT_EQ(header.id, frame.id);
        }
        else
        {
            Log() << "<< " << _name << " not sending dynamic response: id=" << static_cast<int>(header.id);
        }
    }

private:
    Schedule schedule;
    std::unordered_map<LinId, LinFrame> _masterResponses;
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
        Log() << "<< " << _name << " sending goto sleep command";

        controller->GoToSleep();
        _result.gotoSleepSent = true;
        _result.sendTimes.push_back(now);
    }

    void ReceiveFrameStatus(ILinController* /*controller*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

        _result.receivedFrames[frameStatusEvent.status].push_back(frameStatusEvent.frame);
        _result.receiveTimes.push_back(frameStatusEvent.timestamp);
        schedule.ScheduleNextTask();
    }

    void WakeupHandler(ILinController* linController, const LinWakeupEvent& /*wakeupEvent*/)
    {
        Log() << ">> " << _name << " wakeup pulse received";

        linController->WakeupInternal();
        _result.wakeupReceived = true;
        // No further schedule, stop simulation after one cycle
        Stop();
    }

private:
    Schedule schedule;
};

auto MakeSlaveResponses() -> std::unordered_map<LinId, LinFrame>
{
    // Respond to LIN ID 34 dynamically
    LinFrame f34;
    f34.id = 34;
    f34.checksumModel = LinChecksumModel::Enhanced;
    f34.dataLength = 6;
    f34.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};

    std::unordered_map<LinId, LinFrame> result;
    result.insert({f34.id, f34});
    return result;
}

class LinDynamicSlave : public LinNode
{
public:
    LinDynamicSlave(std::string participantName, IParticipant* participant, ILinController* controller,
                    Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, participantName, lifecycleService)
        , _slaveResponses{MakeSlaveResponses()}
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* linController, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

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
        Log() << ">> " << _name << " received goto sleep command";

        // wakeup in 10 ms
        timer.Set(now + 10ms, [linController](std::chrono::nanoseconds /*now*/) {
            linController->Wakeup();
            // The LIN slave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim)
        });
        linController->GoToSleepInternal();
        _result.gotoSleepReceived = true;
    }

    void OnFrameHeader(ILinController* linController,
                       const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& header)
    {
        Log() << ">> " << _name << " received frame header: id=" << (int)header.id << "@" << header.timestamp;

        const auto it = _slaveResponses.find(header.id);
        if (it != _slaveResponses.end())
        {
            const auto& frame = it->second;
            SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            Log() << "<< " << _name << " sending dynamic response: id=" << static_cast<int>(header.id);
            ASSERT_EQ(header.id, frame.id);
        }
        else
        {
            Log() << "!! " << _name << " not sending dynamic response: id=" << static_cast<int>(header.id);
        }
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
    std::unordered_map<LinId, LinFrame> _slaveResponses;
};

class LinSlave : public LinNode
{
public:
    LinSlave(std::string participantName, IParticipant* participant, ILinController* controller,
             Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, participantName, lifecycleService)
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* linController, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

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
        Log() << " Slave GoToSleepHandler";
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

auto MakeDynamicMasterConfig() -> SilKit::Experimental::Services::Lin::LinControllerDynamicConfig
{
    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config;
    config.controllerMode = LinControllerMode::Master;
    config.baudRate = 20'000;
    return config;
}

auto MakeDynamicSlaveConfig() -> SilKit::Experimental::Services::Lin::LinControllerDynamicConfig
{
    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config;
    config.controllerMode = LinControllerMode::Slave;
    config.baudRate = 20'000;
    return config;
}

auto MakeMasterConfig() -> LinControllerConfig
{
    LinControllerConfig config;
    config.controllerMode = LinControllerMode::Master;
    config.baudRate = 20'000;
    return config;
}

auto MakeSlaveConfig() -> LinControllerConfig
{
    LinControllerConfig config;
    config.controllerMode = LinControllerMode::Slave;
    config.baudRate = 20'000;

    LinFrameResponse r16;
    r16.frame.id = 16;
    r16.frame.checksumModel = LinChecksumModel::Classic;
    r16.frame.dataLength = 6;
    r16.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.emplace_back(r16);

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 17
    //  - This LinFrameResponseMode::Unused causes the controller to ignore
    //    this message and not trigger a callback. This is also the default.
    LinFrameResponse r17;
    r17.frame.id = 17;
    r17.frame.checksumModel = LinChecksumModel::Classic;
    r17.frame.dataLength = 6;
    r17.responseMode = LinFrameResponseMode::Unused;
    config.frameResponses.emplace_back(r17);

    // Configure LIN Controller to receive LIN ID 18
    //  - LinChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    LinFrameResponse r18;
    r18.frame.id = 18;
    r18.frame.checksumModel = LinChecksumModel::Classic;
    r18.frame.dataLength = 8;
    r18.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.emplace_back(r18);

    // Configure LIN Controller to receive LIN ID 19
    //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
    LinFrameResponse r19;
    r19.frame.id = 19;
    r19.frame.checksumModel = LinChecksumModel::Enhanced;
    r19.frame.dataLength = 1;
    r19.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.emplace_back(r19);

    // Configure LIN Controller to send a LinFrameResponse for LIN ID 34
    LinFrameResponse r34;
    r34.frame.id = 34;
    r34.frame.checksumModel = LinChecksumModel::Enhanced;
    r34.frame.dataLength = 6;
    r34.frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
    r34.responseMode = LinFrameResponseMode::TxUnconditional;
    config.frameResponses.emplace_back(r34);

    LinFrameResponse r60;
    r60.frame = GoToSleepFrame();
    r60.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.emplace_back(r60);

    return config;
}


class ITest_LinDynamicResponse : public testing::Test
{
protected:
    std::unique_ptr<SimTestHarness> _simTestHarness;
};


//////////////////////////////////////////////////////////////////////
//  Actual Tests
//////////////////////////////////////////////////////////////////////


//! \brief Dynamically respond to a frame header event by sending a previously unconfigured response
TEST_F(ITest_LinDynamicResponse, deferred_simstep_response)
{
    auto participantNames = std::vector<std::string>{"LinMaster", "LinSlave1"};

    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    LinFrame slaveResponseFrame;
    slaveResponseFrame.id = 34;
    slaveResponseFrame.checksumModel = LinChecksumModel::Enhanced;
    slaveResponseFrame.dataLength = 6;
    slaveResponseFrame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};

    LinFrame masterFrame;
    masterFrame.id = 34;
    masterFrame.checksumModel = LinChecksumModel::Enhanced;
    masterFrame.dataLength = 6;

    static constexpr size_t MAX_RESPONSES = 10;
    std::vector<std::chrono::nanoseconds> sentTimes;
    std::vector<std::chrono::nanoseconds> responseTimes;

    //////////////////////////////////////////////////////////////////////
    //  LinMaster
    //////////////////////////////////////////////////////////////////////

    enum class State
    {
        Sending,
        WaitingForResponse,
    } state{State::Sending};

    {
        auto&& participant = _simTestHarness->GetParticipant("LinMaster")->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant("LinMaster")->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant("LinMaster")->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            config.controllerMode = LinControllerMode::Master;
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        linController->AddFrameStatusHandler([&](auto /*linController*/, auto frameStatusEvent) {
            Log() << "Received frameStatusEvent @" << frameStatusEvent.timestamp;
            if (frameStatusEvent.frame == slaveResponseFrame)
            {
                state = State::Sending;
            }
        });

        timeSyncService->SetSimulationStepHandler([&, linController](auto now, auto /*duration*/) {
            if (state == State::Sending)
            {
                Log() << "Sending master frame request @" << now;
                linController->SendFrameHeader(masterFrame.id);
                sentTimes.push_back(now);
                state = State::WaitingForResponse;
            }
        }, 1ms);
    }

    //////////////////////////////////////////////////////////////////////
    //  LinSlave1
    //////////////////////////////////////////////////////////////////////

    std::chrono::nanoseconds headerReceived{0ms};
    {
        auto&& participant = _simTestHarness->GetParticipant("LinSlave1")->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant("LinSlave1")->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant("LinSlave1")->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        timeSyncService->SetSimulationStepHandler(
            [lifecycleService, linController, &headerReceived, &slaveResponseFrame, &responseTimes](auto now,
                                                                                                    auto /*duration*/) {
            //Send response delayed by 2ms
            if (now == (headerReceived + 2ms))
            {
                SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, slaveResponseFrame);
                responseTimes.push_back(now);
                if (responseTimes.size() == MAX_RESPONSES)
                {
                    lifecycleService->Stop("Test done");
                }
                headerReceived = 0ms; //reset
            }
        },
            1ms);

        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(linController,
                                                                   [&](auto&& /*controller*/, auto&& event) {
            Log() << "LinSlave1: Got Frame Header @" << event.timestamp;
            headerReceived = event.timestamp;
        });
    }

    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
}

TEST_F(ITest_LinDynamicResponse, normal_master_dynamic_slave)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");
        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            auto config = MakeMasterConfig();
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

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        linController->AddGoToSleepHandler([slave = slave.get()](auto&& linController, auto&& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, auto /*duration*/) { slave->DoAction(now); }, 1ms);

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [slave = slave.get()](auto&& controller, auto&& event) { slave->OnFrameHeader(controller, event); });
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
            // The LIN slave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim),
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

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 2 frame states on master
    ASSERT_EQ(masterRecvFrames.size(), 2);

    // 1x RX_OK for id 34 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // 5x TX_OK for id 16,17,18,19,60 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 5);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 16);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].id, 17);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].id, 18);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].id, 19);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].id, 60);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 2 frame states on dynamic slave
    ASSERT_EQ(slaveRecvFrames.size(), 2);

    // 5x RX_OK for id 16,17,18,19,60 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 5);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 17);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].id, 18);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].id, 19);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].id, 60);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 1x TX_OK for id 34 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
}

TEST_F(ITest_LinDynamicResponse, normal_master_two_dynamic_slave_collision)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave1", "LinSlave2"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            auto config = MakeMasterConfig();
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

    //////////////////////////////////////////////////////////////////////
    // LIN Slave 1
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave1";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        linController->AddGoToSleepHandler([slave = slave.get()](auto&& linController, auto&& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, auto /*duration*/) { slave->DoAction(now); }, 1ms);

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [slave = slave.get()](auto&& controller, auto&& event) { slave->OnFrameHeader(controller, event); });

        linNodes.emplace_back(std::move(slave));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave 2
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave2";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        linController->AddGoToSleepHandler([slave = slave.get()](auto&& linController, auto&& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, auto /*duration*/) { slave->DoAction(now); }, 1ms);

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [slave = slave.get()](auto&& controller, auto&& event) { slave->OnFrameHeader(controller, event); });

        linNodes.emplace_back(std::move(slave));
    }

    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // The test runs for one schedule cycle with different messages/responses for master/slave

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slave1RecvFrames = linNodes.at(1)->_result.receivedFrames;
    auto&& slave2RecvFrames = linNodes.at(2)->_result.receivedFrames;

    // 2 frame states on master
    ASSERT_EQ(masterRecvFrames.size(), 2);

    // 2x RX_OK for id 34 on master (two repsonses from the two slaves, no collision detection for the trivial simulation)
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 2);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][1].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // 5x TX_OK for id 16,17,18,19,60 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 5);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 16);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].id, 17);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].id, 18);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].id, 19);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].id, 60);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 2 frame states on dynamic slave 1
    ASSERT_EQ(slave1RecvFrames.size(), 2);

    // 6x RX_OK for id 16,17,18,19,34,60 on dynamic slave 1
    ASSERT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK].size(), 6);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 17);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][2].id, 18);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][2].dataLength, 8);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][3].id, 19);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][3].dataLength, 8);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][4].id, 34);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][4].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][4].dataLength, 6);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][4].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][5].id, 60);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][5].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][5].dataLength, 8);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_RX_OK][5].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 1x TX_OK for id 34 on dynamic slave 1
    ASSERT_EQ(slave1RecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slave1RecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // 2 frame states on dynamic slave 2
    ASSERT_EQ(slave2RecvFrames.size(), 2);

    // 6x RX_OK for id 16,17,18,19,34,60 on dynamic slave 2
    ASSERT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK].size(), 6);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 17);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][2].id, 18);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][2].dataLength, 8);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][3].id, 19);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][3].dataLength, 8);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][4].id, 34);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][4].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][4].dataLength, 6);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][4].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][5].id, 60);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][5].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][5].dataLength, 8);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_RX_OK][5].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 1x TX_OK for id 34 on dynamic slave 2
    ASSERT_EQ(slave2RecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slave2RecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // Known Issue (Trivial Simulation):
    //
    // The trivial simulation does not produce RX_ERROR / TX_ERROR when dynamically sent responses would collide.
}

TEST_F(ITest_LinDynamicResponse, dynamic_master_normal_slave)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            auto config = MakeDynamicMasterConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto master = std::make_unique<LinDynamicMaster>(participant, linController, lifecycleService);

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

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [master = master.get()](auto&& controller, auto&& event) { master->OnFrameHeader(controller, event); });

        linNodes.emplace_back(std::move(master));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeSlaveConfig();
            linController->Init(config);
        });

        auto slave = std::make_unique<LinSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });
        linController->AddGoToSleepHandler([slave = slave.get()](auto&& linController, auto&& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, auto /*duration*/) { slave->DoAction(now); }, 1ms);

        linNodes.emplace_back(std::move(slave));
    }

    //Run the test
    auto ok = _simTestHarness->Run(5s);
    EXPECT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // Sim is stopped when master received the wakeup pulse

    for (auto& node : linNodes)
    {
        if (node->_name == "LinSlave")
        {
            EXPECT_EQ(node->_result.numberReceivedInSleep, 0u);
            EXPECT_TRUE(node->_result.gotoSleepReceived)
                << "Assuming node " << node->_name << " has received a GoToSleep";
            // The LIN slave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim),
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

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 2 frame states on master TX_OK,RX_OK
    ASSERT_EQ(masterRecvFrames.size(), 2);

    // 1x RX_OK for id 34 on dynamic master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // 5x TX_OK for id 16,17,18,19,60 on dynamic master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 5);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 16);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].id, 17);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].id, 18);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].id, 19);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].id, 60);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 3 frame states on slave RX_OK,RX_ERROR,TX_OK
    ASSERT_EQ(slaveRecvFrames.size(), 3);

    // 2x RX_OK for id 16,60 on slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 2);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 60);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 2x RX_ERROR for id 18,19 on slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR].size(), 2);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][0].id, 18);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][0].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][0].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][1].id, 19);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][1].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_ERROR][1].data, (std::array<uint8_t, 8>{0}));

    // 1x TX_OK for id 34 on slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // id 17 on slave is ignored
}

TEST_F(ITest_LinDynamicResponse, dynamic_master_dynamic_slave)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            auto config = MakeDynamicMasterConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto master = std::make_unique<LinDynamicMaster>(participant, linController, lifecycleService);

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

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [master = master.get()](auto&& controller, auto&& event) { master->OnFrameHeader(controller, event); });

        linNodes.emplace_back(std::move(master));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });
        linController->AddGoToSleepHandler([slave = slave.get()](auto&& linController, auto&& goToSleepEvent) {
            slave->GoToSleepHandler(linController, goToSleepEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [slave = slave.get()](auto now, auto /*duration*/) { slave->DoAction(now); }, 1ms);

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [slave = slave.get()](auto&& controller, auto&& event) { slave->OnFrameHeader(controller, event); });
        linNodes.emplace_back(std::move(slave));
    }

    //Run the test
    auto ok = _simTestHarness->Run(5s);
    EXPECT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // Sim is stopped when master received the wakeup pulse

    for (auto& node : linNodes)
    {
        if (node->_name == "LinSlave")
        {
            EXPECT_EQ(node->_result.numberReceivedInSleep, 0u);
            EXPECT_TRUE(node->_result.gotoSleepReceived)
                << "Assuming node " << node->_name << " has received a GoToSleep";
            // The LIN slave doesn't receive the wakeup pulse sent by himself in a trivial simulation (without netsim),
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

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 2 frame states on master TX_OK,RX_OK
    ASSERT_EQ(masterRecvFrames.size(), 2);

    // 1x RX_OK for id 34 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // 5x TX_OK for id 16,17,18,19,60 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 5);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 16);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].id, 17);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].id, 18);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].id, 19);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].id, 60);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].dataLength, 8);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_TX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 2 frame states on dynamic slave
    ASSERT_EQ(slaveRecvFrames.size(), 2);

    // 5x RX_OK for id 16,17,18,19,60 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 5);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 16);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 17);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][1].data, (std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].id, 18);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][2].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].id, 19);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][3].data, (std::array<uint8_t, 8>{0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].id, 60);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].checksumModel, LinChecksumModel::Classic);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].dataLength, 8);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_RX_OK][4].data,
              (std::array<uint8_t, 8>{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));

    // 1x TX_OK for id 34 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));
}


class LinMasterCheckResponseResets : public LinNode
{
public:
    LinMasterCheckResponseResets(IParticipant* participant, ILinController* controller,
                                 Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, "LinMaster", lifecycleService)
    {
        timer.Set(10ms, [this](std::chrono::nanoseconds now) { At10Ms(now); });
    }

    void doAction(std::chrono::nanoseconds now)
    {
        timer.ExecuteAction(now);
    }

    void At10Ms(std::chrono::nanoseconds now)
    {
        SendFrame_34(now);
        timer.Set(20ms, [this](std::chrono::nanoseconds now) { At20Ms(now); });
    }

    void At20Ms(std::chrono::nanoseconds now)
    {
        SendFrame_34(now);
        timer.Set(30ms, [this](std::chrono::nanoseconds now) { At30Ms(now); });
    }

    void At30Ms(std::chrono::nanoseconds)
    {
        _lifecycleService->Stop("done");
    }

    void SendFrame_34(std::chrono::nanoseconds now)
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = SilKit::Services::Lin::LinChecksumModel::Enhanced;
        frame.dataLength = 6;

        _result.sendTimes.push_back(now);
        controller->SendFrame(frame, LinFrameResponseType::SlaveResponse);
        Log() << "<< LIN LinFrame Header sent for ID=" << static_cast<unsigned int>(frame.id) << " now=" << now;
    }

    void ReceiveFrameStatus(ILinController* /*controller*/, const LinFrameStatusEvent& frameStatusEvent)
    {
        switch (frameStatusEvent.status)
        {
        case LinFrameStatus::LIN_RX_OK:
            break; // good case, no need to warn
        case LinFrameStatus::LIN_TX_OK:
            break; // good case, no need to warn
        default:
            Log() << "WARNING: LIN transmission failed!";
        }

        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

        _result.receivedFrames[frameStatusEvent.status].push_back(frameStatusEvent.frame);
        _result.receiveTimes.push_back(frameStatusEvent.timestamp);
    }

private:
    Timer timer;
};

class LinDynamicSlaveCheckResponseResets : public LinNode
{
public:
    LinDynamicSlaveCheckResponseResets(std::string participantName, IParticipant* participant,
                                       ILinController* controller, Orchestration::ILifecycleService* lifecycleService)
        : LinNode(participant, controller, participantName, lifecycleService)
        , _slaveResponses{MakeSlaveResponses()}
    {
    }

    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;
        timer.ExecuteAction(now);
    }

    void FrameStatusHandler(ILinController* linController, const LinFrameStatusEvent& frameStatusEvent)
    {
        Log() << ">> " << _name << " " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
              << " timestamp=" << frameStatusEvent.timestamp;

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

    void OnFrameHeader(ILinController* linController,
                       const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& header)
    {
        Log() << ">> Received Frame Header: id=" << (int)header.id << "@" << header.timestamp;

        const auto it = _slaveResponses.find(header.id);
        if (it != _slaveResponses.end())
        {
            const auto& frame = it->second;
            SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            Log() << "<< Sending dynamic response: id=" << static_cast<int>(header.id);
            ASSERT_EQ(header.id, frame.id);

            // reset the response after the first time sending it
            _slaveResponses.erase(it);
        }
        else
        {
            Log() << "!! Not sending dynamic response: id=" << static_cast<int>(header.id);
        }
    }

private:
    Timer timer;
    std::chrono::nanoseconds now{0ns};
    std::unordered_map<LinId, LinFrame> _slaveResponses;
};

TEST_F(ITest_LinDynamicResponse, normal_master_dynamic_slave_response_resets)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            Log() << "Initializing " << participantName;
            auto config = MakeMasterConfig();
            LinFrameResponse r34{};
            r34.frame.id = 34;
            r34.frame.checksumModel = LinChecksumModel::Enhanced;
            r34.frame.dataLength = 6;
            r34.responseMode = LinFrameResponseMode::Rx;
            config.frameResponses.emplace_back(r34);
            linController->Init(config);
        });

        auto master = std::make_unique<LinMasterCheckResponseResets>(participant, linController, lifecycleService);

        linController->AddFrameStatusHandler(
            [master = master.get()](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
            master->ReceiveFrameStatus(linController, frameStatusEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [master = master.get(), participantName](auto now, std::chrono::nanoseconds /*duration*/) {
            Log() << participantName << " now=" << now.count() << "ns";
            master->doAction(now);
        }, 1ms);

        linNodes.emplace_back(std::move(master));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            Log() << " Initializing " << participantName;
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlaveCheckResponseResets>(participantName, participant, linController,
                                                                          lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        timeSyncService->SetSimulationStepHandler([slave = slave.get()](auto now, auto /*duration*/) {
            Log() << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms";
            slave->DoAction(now);
        }, 1ms);

        // Dynamic Response API starts here
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            linController,
            [slave = slave.get()](auto&& controller, auto&& event) { slave->OnFrameHeader(controller, event); });

        linNodes.emplace_back(std::move(slave));
    }


    //Run the test
    auto ok = _simTestHarness->Run(5000s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // Sim is stopped after master sent second frame

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;

    // 1 frame states on master
    ASSERT_EQ(masterRecvFrames.size(), 1);

    // 1x RX_OK for id 34 on master
    ASSERT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 1);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel,
              SilKit::Services::Lin::LinChecksumModel::Enhanced);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(masterRecvFrames[LinFrameStatus::LIN_RX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // Known Issue (Trivial Simulation):
    //
    // Trivial simulation does not produce the LIN_RX_NO_RESPONSE for frames that are _not_ responded to dynamically.

    // 1 frame states on dynamic slave
    ASSERT_EQ(slaveRecvFrames.size(), 1);

    // 1x TX_OK for id 34 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 1);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel,
              SilKit::Services::Lin::LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4}));

    // Known Issue (Trivial Simulation):
    //
    // Trivial simulation does not produce the LIN_RX_NO_RESPONSE for frames that are _not_ responded to dynamically.
}

TEST_F(ITest_LinDynamicResponse, normal_master_dynamic_slave_out_of_band_response_does_nothing)
{
    std::vector<std::string> participantNames = {"LinMaster", "LinSlave", "LinSlaveDynamic"};
    _simTestHarness = std::make_unique<SimTestHarness>(participantNames, "silkit://localhost:0", false);

    std::vector<std::unique_ptr<LinNode>> linNodes;
    //Create a simulation setup with 2 participants

    //////////////////////////////////////////////////////////////////////
    // LIN Master
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinMaster";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");
        lifecycleService->SetCommunicationReadyHandler([participantName, linController]() {
            auto config = MakeMasterConfig();
            linController->Init(config);
        });

        auto master = std::make_unique<LinMaster>(participant, linController, lifecycleService);

        linController->AddFrameStatusHandler(
            [master = master.get()](ILinController* linController, const LinFrameStatusEvent& frameStatusEvent) {
            master->ReceiveFrameStatus(linController, frameStatusEvent);
        });

        timeSyncService->SetSimulationStepHandler(
            [lifecycleService, participantName](auto now, std::chrono::nanoseconds /*duration*/) {
            if (now == 30ms)
            {
                lifecycleService->Stop("done");
            }
        }, 1ms);

        linNodes.emplace_back(std::move(master));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlave";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        timeSyncService->SetSimulationStepHandler([linController](auto now, auto /*duration*/) {
            Log() << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms";

            LinFrame frame{};
            frame.id = 34;
            frame.checksumModel = SilKit::Services::Lin::LinChecksumModel::Enhanced;
            frame.dataLength = 6;
            frame.data = std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 0, 0};

            if (now == 10ms || now == 20ms)
            {
                SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            }
        }, 1ms);

        linNodes.emplace_back(std::move(slave));
    }

    //////////////////////////////////////////////////////////////////////
    // LIN Slave
    //////////////////////////////////////////////////////////////////////
    {
        const std::string participantName = "LinSlaveDynamic";
        auto&& participant = _simTestHarness->GetParticipant(participantName)->Participant();
        auto&& lifecycleService = _simTestHarness->GetParticipant(participantName)->GetOrCreateLifecycleService();
        auto&& timeSyncService = _simTestHarness->GetParticipant(participantName)->GetOrCreateTimeSyncService();
        auto&& linController = participant->CreateLinController("LinController1", "LIN_1");

        lifecycleService->SetCommunicationReadyHandler([linController]() {
            auto config = MakeDynamicSlaveConfig();
            SilKit::Experimental::Services::Lin::InitDynamic(linController, config);
        });

        auto slave = std::make_unique<LinDynamicSlave>(participantName, participant, linController, lifecycleService);

        linController->AddFrameStatusHandler([slave = slave.get()](auto&& linController, auto&& frameStatusEvent) {
            slave->FrameStatusHandler(linController, frameStatusEvent);
        });

        timeSyncService->SetSimulationStepHandler([](auto now, auto /*duration*/) {
            Log() << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms";
        }, 1ms);

        linNodes.emplace_back(std::move(slave));
    }

    //Run the test
    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // The test runs for one schedule cycle with different messages/responses for master/slave

    auto&& masterRecvFrames = linNodes.at(0)->_result.receivedFrames;
    auto&& slaveRecvFrames = linNodes.at(1)->_result.receivedFrames;
    auto&& slaveDynamicRecvFrames = linNodes.at(2)->_result.receivedFrames;

    // 2 frame states on master
    ASSERT_EQ(masterRecvFrames.size(), 0);

    // 2 frame states on dynamic slave
    ASSERT_EQ(slaveRecvFrames.size(), 1);

    // 1x TX_OK for id 34 on dynamic slave
    ASSERT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK].size(), 2);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][0].data, (std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 0, 0}));
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][1].id, 34);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][1].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][1].dataLength, 6);
    EXPECT_EQ(slaveRecvFrames[LinFrameStatus::LIN_TX_OK][1].data, (std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 0, 0}));

    // 2 frame states on dynamic slave
    ASSERT_EQ(slaveDynamicRecvFrames.size(), 1);

    // 1x TX_OK for id 34 on dynamic slave
    ASSERT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK].size(), 2);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][0].id, 34);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][0].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][0].dataLength, 6);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][0].data,
              (std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 0, 0}));
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][1].id, 34);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][1].checksumModel, LinChecksumModel::Enhanced);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][1].dataLength, 6);
    EXPECT_EQ(slaveDynamicRecvFrames[LinFrameStatus::LIN_RX_OK][1].data,
              (std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 0, 0}));

    // Known Issue (Trivial Simulation):
    //
    // The node using SendDynamicResponse always receives a FrameStatusHandler with LIN_TX_OK, even if no frame was
    // requested via SendFrameHeader or SendFrame(..., SlaveResponse)
}

} //end namespace
