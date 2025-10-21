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
    std::unordered_map<LinId, LinFrame> _slaveResponses;

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
        UpdateDynamicResponseTo34();

        _linController->AddFrameStatusHandler(
            [this](ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent) {
            // On a TX acknowledge for ID 34, update the TxBuffer for the next transmission
            if (frameStatusEvent.frame.id == 34)
            {
                UpdateDynamicResponseTo34();
            }

            std::stringstream ss;
            ss << "Received " << frameStatusEvent.frame << " status=" << frameStatusEvent.status
               << " timestamp=" << frameStatusEvent.timestamp;

            GetLogger()->Info(ss.str());
        });

        _linController->AddGoToSleepHandler(
            [this](ILinController* /*linController*/, const LinGoToSleepEvent& /*goToSleepEvent*/) {
            std::stringstream ss;
            ss << "Received go-to-sleep command; entering sleep mode.";
            GetLogger()->Info(ss.str());

            // Wakeup in 10 ms
            _timer.Set(_now + 10ms, [this](std::chrono::nanoseconds now) {
                std::stringstream ss;
                ss << "Sending Wakeup pulse @" << now;
                GetLogger()->Info(ss.str());

                _linController->Wakeup();
            });
            _linController->GoToSleepInternal();
        });

        _linController->AddWakeupHandler([this](ILinController* /*linController*/, const LinWakeupEvent& wakeupEvent) {
            std::stringstream ss;
            ss << "LIN Slave received wakeup pulse; direction=" << wakeupEvent.direction
               << "; Entering normal operation mode.";
            GetLogger()->Info(ss.str());

            // No need to set the controller status if we sent the wakeup
            if (wakeupEvent.direction == SilKit::Services::TransmitDirection::RX)
            {
                _linController->WakeupInternal();
            }
        });

        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            _linController, [this](ILinController* /*linController*/,
                                   const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& headerEvent) {
            {
                std::stringstream ss;
                ss << "Received frame header: id=" << (int)headerEvent.id << " @" << headerEvent.timestamp;
                GetLogger()->Info(ss.str());
            }

            const auto it = _slaveResponses.find(headerEvent.id);
            if (it != _slaveResponses.end())
            {
                std::stringstream ss;
                ss << "Sending dynamic response: id=" << static_cast<int>(headerEvent.id);
                GetLogger()->Info(ss.str());

                const auto& frame = it->second;
                SilKit::Experimental::Services::Lin::SendDynamicResponse(_linController, frame);
            }
            else
            {
                std::stringstream ss;
                ss << "!! Not sending dynamic response: id=" << static_cast<int>(headerEvent.id);
                GetLogger()->Info(ss.str());
            }
        });
    }

    void InitControllers() override
    {
        SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config{};
        config.controllerMode = LinControllerMode::Slave;
        config.baudRate = 20'000;

        SilKit::Experimental::Services::Lin::InitDynamic(_linController, config);
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

    void UpdateDynamicResponseTo34()
    {
        LinFrame f34{};
        f34.id = 34;
        f34.checksumModel = LinChecksumModel::Enhanced;
        f34.dataLength = 6;
        f34.data = {static_cast<uint8_t>(rand() % 10), 0, 0, 0, 0, 0, 0, 0};
        _slaveResponses[f34.id] = f34;
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
