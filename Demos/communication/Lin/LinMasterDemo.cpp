// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "LinDemoCommon.hpp"

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
            ss << "Received " << frameStatusEvent.frame << ", status=" << frameStatusEvent.status;
            GetLogger()->Info(ss.str());
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
        });

        _schedule = std::make_unique<LinDemoCommon::Schedule>(
            std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>>{
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_16(); }},
                {20ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_17(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_18(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_19(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_34(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); }}});
    }

    void InitControllers() override
    {
        LinControllerConfig config;
        config.controllerMode = LinControllerMode::Master;
        config.baudRate = 20'000;
        _linController->Init(config);
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        _now = now;
        _schedule->ExecuteTask(now);
    }

    void DoWorkAsync() override
    {
        _schedule->ExecuteTask(_now);
        _now += 1ms;
    }

    // LinMaster schedule

    void SendFrameIfOperational(const LinFrame& linFrame, LinFrameResponseType responseType)
    {
        const auto linId{static_cast<unsigned>(linFrame.id)};

        if (_linController->Status() != LinControllerStatus::Operational)
        {
            std::stringstream ss;
            ss << "LIN Frame with ID=" << linId << " not sent, since the controller is not operational";
            GetLogger()->Warn(ss.str());
            return;
        }

        _linController->SendFrame(linFrame, responseType);

        std::stringstream ss;
        if (responseType == LinFrameResponseType::SlaveResponse)
        {
            ss << "LIN frame Header sent for ID=" << linId;
        }
        else
        {
            ss << "LIN frame sent with ID=" << linId;
        }
        GetLogger()->Info(ss.str());
    }

    void SendFrame_16()
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_17()
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_18()
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_19()
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_34()
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;

        SendFrameIfOperational(frame, LinFrameResponseType::SlaveResponse);
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
