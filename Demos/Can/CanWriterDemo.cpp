// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "CanDemoCommon.hpp"

using namespace std::chrono_literals;

class CanWriter: public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase; 

private:
    ICanController* _canController{nullptr};

    std::string networkName = "CAN1";
    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "n", networkName, "[--network <name>]", "-n, --network: Name of the CAN network to use. Defaults to '" + networkName + "'.");
    }

    void EvaluateCommandLineArgs() override 
    {
        networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
    }

    void CreateControllers() override
    {
        _canController = GetParticipant()->CreateCanController("CanController1", networkName);

        _canController->AddFrameTransmitHandler([this](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
            CanDemoCommon::FrameTransmitHandler(ack, GetLogger());
        });
        _canController->AddFrameHandler([this](ICanController* /*ctrl*/, const CanFrameEvent& frameEvent) {
            CanDemoCommon::FrameHandler(frameEvent, GetLogger());
        });
    }

    void InitControllers() override
    {
        _canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
        _canController->Start();
    }

    void SendFrame()
    {
        // Count up message id per frame
        static uint64_t messageId = 0;
        messageId++;

        // Build a CAN frame
        CanFrame canFrame{};
        canFrame.canId = 3;

        // Cycle between normal and FD frames
        canFrame.flags = static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf) // FD Format Indicator
                            | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs); // Bit Rate Switch (for FD Format only)
        

        // Build a payload with the message Id
        std::stringstream payloadBuilder;
        payloadBuilder << "CAN " << messageId;
        auto payloadStr = payloadBuilder.str();
        std::vector<uint8_t> payloadBytes(payloadStr.begin(), payloadStr.end());
        canFrame.dataField = payloadBytes;
        canFrame.dlc = static_cast<uint16_t>(canFrame.dataField.size());

        // Log
        std::stringstream buffer;
        buffer << "Send CAN FD frame, canId=" << canFrame.canId << ", data='" << payloadStr
               << "' ";
        GetLogger()->Info(buffer.str());

        // Send
        _canController->SendFrame(std::move(canFrame));
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
    app.SetupCommandLineArgs(argc, argv);

    return app.Run();
}

