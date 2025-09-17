// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "LinDemoCommon.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

class LinSlave : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    ILinController* _linController{nullptr};
    LinDemoCommon::Timer _timer;
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
            // On a TX acknowledge for ID 34, update the TxBuffer for the next transmission
            if (frameStatusEvent.frame.id == 34)
            {
                LinFrame frame34;
                frame34.id = 34;
                frame34.checksumModel = LinChecksumModel::Enhanced;
                frame34.dataLength = 8;
                frame34.data = {static_cast<uint8_t>(rand() % 10), 0, 0, 0, 0, 0, 0, 0};
                _linController->UpdateTxBuffer(frame34);
            }

            std::stringstream ss;
            ss << "Received " << frameStatusEvent.frame << " status=" << frameStatusEvent.status;

            GetLogger()->Info(ss.str());
        });

        _linController->AddGoToSleepHandler(
            [this](ILinController* /*linController*/, const LinGoToSleepEvent& /*goToSleepEvent*/) {
            std::stringstream ss;
            ss << "Received go-to-sleep command; entering sleep mode.";
            GetLogger()->Info(ss.str());

            // Wakeup in 15 ms
            _timer.Set(_now + 15ms, [this](std::chrono::nanoseconds now) {
                std::stringstream ss;
                ss << "Sending Wakeup pulse @" << now;
                GetLogger()->Info(ss.str());

                _linController->Wakeup();
            });
            _linController->GoToSleepInternal();
        });

        _linController->AddWakeupHandler([this](ILinController* /*linController*/, const LinWakeupEvent& wakeupEvent) {
            std::stringstream ss;
            ss << "Received Wakeup pulse, direction=" << wakeupEvent.direction << "; Entering normal operation mode.";
            GetLogger()->Info(ss.str());

            // No need to set the controller status if we sent the wakeup
            if (wakeupEvent.direction == SilKit::Services::TransmitDirection::RX)
            {
                _linController->WakeupInternal();
            }
        });
    }

    void InitControllers() override
    {
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
        response_34.frame.dataLength = 8;
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

        _linController->Init(config);
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        _now = now;
        _timer.ExecuteIfDue(now);
    }

    void DoWorkAsync() override
    {
        _timer.ExecuteIfDue(_now);
        _now += 1ms;
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "LinSlave";
    LinSlave app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Lin: Slave Node");

    return app.Run();
}
