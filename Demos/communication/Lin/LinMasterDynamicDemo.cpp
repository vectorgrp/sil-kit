// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "LinDemoCommon.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

class LinMaster : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    ILinController* _linController{nullptr};
    std::unique_ptr<LinDemoCommon::Schedule> _schedule;
    std::chrono::nanoseconds _now{0ns};
    std::string _networkName = "LIN1";
    std::unordered_map<LinId, LinFrame> _masterResponses;

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the LIN network to use.", "Defaults to '" + _networkName + "'."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
    }

    void CreateControllers() override
    {
        _linController = GetParticipant()->CreateLinController("LinController1", _networkName);

        _linController->AddFrameStatusHandler(
            [this](ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent) {
            switch (frameStatusEvent.status)
            {
            case LinFrameStatus::LIN_RX_OK:
                break; // good case, no need to warn
            case LinFrameStatus::LIN_TX_OK:
                break; // good case, no need to warn
            default:
                std::stringstream ss;
                ss << "LIN transmission failed!";
                GetLogger()->Warn(ss.str());
            }

            std::stringstream ss;
            ss << "Received FrameStatus for " << frameStatusEvent.frame << ", status=" << frameStatusEvent.status;
            GetLogger()->Info(ss.str());

            _schedule->ScheduleNextTask();
        });

        _linController->AddWakeupHandler([this](ILinController* /*linController*/, const LinWakeupEvent& wakeupEvent) {
            if (_linController->Status() != LinControllerStatus::Sleep)
            {
                std::stringstream ss;
                ss << "Received Wakeup pulse while LinControllerStatus is " << _linController->Status() << ".";
                GetLogger()->Warn(ss.str());
            }

            std::stringstream ss;
            ss << "Received Wakeup pulse, direction=" << wakeupEvent.direction;
            GetLogger()->Info(ss.str());

            _linController->WakeupInternal();

            _schedule->ScheduleNextTask();
        });

        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            _linController, [this](ILinController* linController,
                                   const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& headerEvent) {
            {
                std::stringstream ss;
                ss << "Received frame header: id=" << (int)headerEvent.id;
                GetLogger()->Info(ss.str());
            }

            const auto it = _masterResponses.find(headerEvent.id);
            if (it != _masterResponses.end())
            {
                std::stringstream ss;
                ss << "Sending dynamic response: id=" << static_cast<int>(headerEvent.id);
                GetLogger()->Info(ss.str());

                const auto& frame = it->second;
                SilKit::Experimental::Services::Lin::SendDynamicResponse(linController, frame);
            }
            else
            {
                std::stringstream ss;
                ss << "!! Not sending dynamic response: id=" << static_cast<int>(headerEvent.id);
                GetLogger()->Info(ss.str());
            }
        });

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

        _schedule = std::make_unique<LinDemoCommon::Schedule>(
            std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>>{
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrameHeader(16); }},
                {20ms, [this](std::chrono::nanoseconds /*now*/) { SendFrameHeader(17); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrameHeader(18); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrameHeader(19); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrameHeader(34); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); }}},
            false);
    }

    void InitControllers() override
    {
        SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config;
        config.controllerMode = LinControllerMode::Master;
        config.baudRate = 20'000;

        SilKit::Experimental::Services::Lin::InitDynamic(_linController, config);
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        _now = now;
        DoWork();
    }

    void DoWorkAsync() override
    {
        DoWork();
        _now += 1ms;
    }

    void DoWork()
    {
        if (_linController->Status() != LinControllerStatus::Operational)
            return;
        _schedule->ExecuteTask(_now);
    }

    // LinMaster schedule

    void SendFrameHeader(LinId linId)
    {
        _linController->SendFrameHeader(linId);
        std::stringstream ss;
        ss << "Sending LIN frame header, ID=" << static_cast<uint16_t>(linId) << " @" << _now;
        GetLogger()->Info(ss.str());
    }

    void GoToSleep()
    {
        std::stringstream ss;
        ss << "Sending Go-To-Sleep command and entering sleep state";
        GetLogger()->Info(ss.str());

        _linController->GoToSleep();
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "LinMaster";
    LinMaster app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Lin: Master Node");

    return app.Run();
}
