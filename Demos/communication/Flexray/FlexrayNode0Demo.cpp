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
    std::string _invalidCase{};

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the Flexray network to use.", "Defaults to '" + _networkName + "'."});

        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "invalid", "I", _invalidCase, "-I, --invalid <case>",
            std::vector<std::string>{"Send deliberately malformed FlexRay frames instead of valid ones,",
                                     "to exercise a receiver's frame validation.",
                                     "The case is applied to the last TX buffer, never to the key slot buffer.",
                                     "'list' prints the cases with their expected reason and exits.",
                                     "Cases: " + FlexrayDemoCommon::InvalidFrames::NameList() + "."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
        _invalidCase = GetCommandLineParser()->Get<CommandlineParser::Option>("invalid").Value();

        if (_invalidCase == "list")
        {
            std::cout << std::endl << "Malformed FlexRay frame cases:" << std::endl << std::endl;
            for (const auto& c : FlexrayDemoCommon::InvalidFrames::All())
            {
                std::cout << "  " << std::setw(14) << std::left << c.name << "  " << c.expectedReason << std::endl;
            }
            std::cout << std::endl;
            std::exit(0);
        }

        if (!_invalidCase.empty() && FlexrayDemoCommon::InvalidFrames::Find(_invalidCase) == nullptr)
        {
            throw std::runtime_error("Unknown --invalid case '" + _invalidCase + "'. Known cases: "
                                     + FlexrayDemoCommon::InvalidFrames::NameList() + ", list.");
        }
    }

    void CreateControllers() override
    {
        auto flexrayController = GetParticipant()->CreateFlexrayController("FlexrayController1", _networkName);

        FlexrayControllerConfig config = FlexrayDemoCommon::MakeControllerConfig();

        // The specific buffer configs for this node
        std::vector<FlexrayTxBufferConfig> bufferConfigs;
        FlexrayTxBufferConfig baseBufferCfg{};
        baseBufferCfg.offset = 0;
        baseBufferCfg.repetition = 1;
        baseBufferCfg.hasPayloadPreambleIndicator = false;
        baseBufferCfg.headerCrc = 5;
        baseBufferCfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::AB;
            cfg.slotId = 40;
            bufferConfigs.push_back(cfg);
        }
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::A;
            cfg.slotId = 41;
            bufferConfigs.push_back(cfg);
        }
        {
            FlexrayTxBufferConfig cfg = baseBufferCfg;
            cfg.channels = FlexrayChannel::B;
            cfg.slotId = 42;
            bufferConfigs.push_back(cfg);
        }
        // Apply the selected malformed-frame case to the last buffer. Never to buffer 0 - that is the key slot
        // buffer FlexRay startup and sync depend on, and breaking it means nothing is ever transmitted.
        const auto* invalidCase = _invalidCase.empty() ? nullptr : FlexrayDemoCommon::InvalidFrames::Find(_invalidCase);
        if (invalidCase != nullptr && invalidCase->ApplyToBufferConfig)
        {
            invalidCase->ApplyToBufferConfig(bufferConfigs.back());

            std::stringstream ss;
            ss << "Malformed FlexRay TX buffer '" << invalidCase->name << "' on buffer " << bufferConfigs.size() - 1
               << ": slotId=" << bufferConfigs.back().slotId << ", headerCrc=" << bufferConfigs.back().headerCrc
               << ", channels=" << bufferConfigs.back().channels << " - expecting reason '"
               << invalidCase->expectedReason << "'";
            GetLogger()->Info(ss.str());
        }

        config.bufferConfigs = bufferConfigs;

        // The specific keyslotID for this node
        config.nodeParams.pKeySlotId = 40;

        _flexrayNode = std::make_unique<FlexrayDemoCommon::FlexrayNode>(flexrayController, std::move(config),
                                                                        GetLogger(), invalidCase);
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
    Arguments args{};
    args.participantName = "Node0";

    FlexrayNode0 app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Flexray: Node0 of a two-node Flexray system",
                             {ApplicationBase::DefaultArg::Async});

    return app.Run();
}
