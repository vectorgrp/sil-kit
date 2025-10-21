// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "CanDemoCommon.hpp"

class CanReader : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    ICanController* _canController{nullptr};
    std::string _networkName = "CAN1";
    bool _printHex{false};

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the CAN network to use.", "Defaults to '" + _networkName + "'."});

        GetCommandLineParser()->Add<CommandlineParser::Flag>(
            "hex", "H", "-H, --hex",
            std::vector<std::string>{"Print the CAN payloads in hexadecimal format.",
                                     "Otherwise, the payloads are interpreted as strings."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
        _printHex = GetCommandLineParser()->Get<CommandlineParser::Flag>("hex").Value();
    }

    void CreateControllers() override
    {
        _canController = GetParticipant()->CreateCanController("CanController1", _networkName);

        _canController->AddFrameTransmitHandler([this](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
            CanDemoCommon::FrameTransmitHandler(ack, GetLogger());
        });
        _canController->AddFrameHandler([this](ICanController* /*ctrl*/, const CanFrameEvent& frameEvent) {
            CanDemoCommon::FrameHandler(frameEvent, GetLogger(), _printHex);
        });
    }

    void InitControllers() override
    {
        _canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
        _canController->Start();
    }

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override
    {
        // NOP
    }

    void DoWorkAsync() override
    {
        // NOP
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "CanReader";
    args.duration = 5ms;
    CanReader app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Can: Log received frames");

    return app.Run();
}
