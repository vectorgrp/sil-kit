// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "EthernetDemoCommon.hpp"

class EthernetReader : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IEthernetController* _ethernetController{nullptr};
    std::string _networkName = "Eth1";
    bool _printHex{false};

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the Ethernet network to use.", "Defaults to '" + _networkName + "'."});

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
        _ethernetController = GetParticipant()->CreateEthernetController("EthernetController1", _networkName);

        _ethernetController->AddFrameTransmitHandler(
            [this](IEthernetController* /*ctrl*/, const EthernetFrameTransmitEvent& ack) {
            EthernetDemoCommon::FrameTransmitHandler(ack, GetLogger());
        });
        _ethernetController->AddFrameHandler(
            [this](IEthernetController* /*ctrl*/, const EthernetFrameEvent& frameEvent) {
            EthernetDemoCommon::FrameHandler(frameEvent, GetLogger(), _printHex);
        });
    }

    void InitControllers() override
    {
        _ethernetController->Activate();
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
    args.participantName = "EthernetReader";
    EthernetReader app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Ethernet: Log received frames");

    return app.Run();
}
