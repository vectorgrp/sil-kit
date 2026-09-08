// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "CanDemoCommon.hpp"

class CanWriter : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    ICanController* _canController{nullptr};
    std::string _networkName = "CAN1";
    bool _printHex{false};
    int _frameId = 0;
    std::string _invalidCase{};
    size_t _invalidIndex{0};

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the CAN network to use.", "Defaults to '" + _networkName + "'."});

        GetCommandLineParser()->Add<CommandlineParser::Flag>("hex", "H", "-H, --hex",
                                                             std::vector<std::string>{"Print the CAN payload as hex."});

        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "invalid", "I", _invalidCase, "-I, --invalid <case>",
            std::vector<std::string>{"Send deliberately malformed CAN frames instead of valid ones,",
                                     "to exercise a receiver's frame validation.",
                                     "'all' cycles through every case, one per tick.",
                                     "'list' prints the cases with their expected reason and exits.",
                                     "Cases: " + CanDemoCommon::InvalidFrames::NameList() + "."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
        _printHex = GetCommandLineParser()->Get<CommandlineParser::Flag>("hex").Value();
        _invalidCase = GetCommandLineParser()->Get<CommandlineParser::Option>("invalid").Value();

        if (_invalidCase == "list")
        {
            std::cout << std::endl << "Malformed CAN frame cases:" << std::endl << std::endl;
            for (const auto& c : CanDemoCommon::InvalidFrames::All())
            {
                std::cout << "  " << std::setw(10) << std::left << c.name << "  " << c.expectedReason << std::endl;
            }
            std::cout << std::endl;
            std::exit(0);
        }

        if (!_invalidCase.empty() && _invalidCase != "all"
            && CanDemoCommon::InvalidFrames::Find(_invalidCase) == nullptr)
        {
            throw std::runtime_error("Unknown --invalid case '" + _invalidCase + "'. Known cases: "
                                     + CanDemoCommon::InvalidFrames::NameList() + ", all, list.");
        }
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

    //! Send one deliberately malformed frame, see CanDemoCommon::InvalidFrames.
    void SendInvalidFrame()
    {
        const auto& cases = CanDemoCommon::InvalidFrames::All();
        const CanDemoCommon::InvalidFrames::Case* invalidCase = nullptr;
        if (_invalidCase == "all")
        {
            invalidCase = &cases[_invalidIndex++ % cases.size()];
        }
        else
        {
            invalidCase = CanDemoCommon::InvalidFrames::Find(_invalidCase);
        }

        // Start from a frame that passes every check, then let the case violate exactly one of them.
        CanFrame canFrame{};
        canFrame.canId = 3;
        canFrame.flags = 0;
        canFrame.dlc = 0;
        std::vector<uint8_t> payloadBytes;

        invalidCase->Build(canFrame, payloadBytes);
        canFrame.dataField = payloadBytes;

        std::stringstream ss;
        ss << "Sending malformed CAN frame '" << invalidCase->name << "': canId=" << canFrame.canId << ", flags=0x"
           << std::hex << canFrame.flags << std::dec << ", dlc=" << canFrame.dlc
           << ", datasize=" << canFrame.dataField.size() << " - expecting reason '" << invalidCase->expectedReason
           << "'";
        GetLogger()->Info(ss.str());

        _canController->SendFrame(canFrame);
    }

    void SendFrame()
    {
        _frameId++;

        if (!_invalidCase.empty())
        {
            SendInvalidFrame();
            return;
        }

        // Build a CAN FD frame
        CanFrame canFrame{};
        canFrame.canId = 3;
        canFrame.flags = static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)    // FD Format Indicator
                         | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs); // Bit Rate Switch (for FD Format only)

        // Build a payload with the frame Id
        std::stringstream payloadBuilder;
        payloadBuilder << "CAN " << _frameId % 10000;
        auto payloadStr = payloadBuilder.str();
        std::vector<uint8_t> payloadBytes(payloadStr.begin(), payloadStr.end());
        canFrame.dataField = payloadBytes;
        canFrame.dlc = static_cast<uint16_t>(canFrame.dataField.size());

        // Log
        std::stringstream ss;
        ss << "Sending CAN FD frame, canId=" << canFrame.canId << ", data=";
        if (_printHex)
        {
            ss << "[" << Util::AsHexString(canFrame.dataField).WithSeparator(" ") << "]";
        }
        else
        {
            ss << "'" << std::string(canFrame.dataField.begin(), canFrame.dataField.end()) << "'";
        }
        GetLogger()->Info(ss.str());

        // Send
        _canController->SendFrame(canFrame);
    }

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override
    {
        SendFrame();
    }

    void DoWorkAsync() override
    {
        SendFrame();
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "CanWriter";
    args.duration = 5ms;
    CanWriter app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Can: Send CanFd frames");

    return app.Run();
}
