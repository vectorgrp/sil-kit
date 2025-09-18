// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "FlexrayDemoCommon.hpp"

using namespace SilKit::Services::Flexray;

class FlexrayNode0 : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    std::unique_ptr<FlexrayDemoCommon::FlexrayNode> _flexrayNode;
    std::string _networkName = "PowerTrain1";

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the Flexray network to use.", "Defaults to '" + _networkName + "'."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
    }

    void CreateControllers() override
    {
        auto flexrayController = GetParticipant()->CreateFlexrayController("FlexrayController1", _networkName);

        FlexrayControllerConfig config = FlexrayDemoCommon::MakeControllerConfig();

        // The specific buffer configs for this node
        std::vector<FlexrayTxBufferConfig> bufferConfigs;
        FlexrayTxBufferConfig baseBufferCfg;
        baseBufferCfg.offset = 0;
        baseBufferCfg.repetition = 1;
        baseBufferCfg.hasPayloadPreambleIndicator = false;
        baseBufferCfg.headerCrc = 5;
        baseBufferCfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::AB;
            cfg.slotId = 60;
            bufferConfigs.push_back(cfg);
        }
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::A;
            cfg.slotId = 61;
            bufferConfigs.push_back(cfg);
        }
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::B;
            cfg.slotId = 62;
            bufferConfigs.push_back(cfg);
        }
        config.bufferConfigs = bufferConfigs;

        // The specific keyslotID for this node
        config.nodeParams.pKeySlotId = 60;

        _flexrayNode =
            std::make_unique<FlexrayDemoCommon::FlexrayNode>(flexrayController, std::move(config), GetLogger());
    }

    void InitControllers() override
    {
        // Configuration is done via POC state
    }

    void DoWorkAsync() override
    {
        // No async mode for flexray
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        _flexrayNode->DoAction(now);
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "Node1";
    FlexrayNode0 app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Flexray: Node1 of a two-node Flexray system",
                             {ApplicationBase::DefaultArg::Async});

    return app.Run();
}
