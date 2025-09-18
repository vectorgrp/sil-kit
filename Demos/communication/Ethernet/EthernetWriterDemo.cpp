// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "EthernetDemoCommon.hpp"

class EthernetWriter : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IEthernetController* _ethernetController{nullptr};
    std::string _networkName = "Eth1";
    bool _printHex{false};
    int _frameId = 0;

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

    std::vector<uint8_t> CreateFrame(const EthernetDemoCommon::EthernetMac& destinationAddress,
                                     const EthernetDemoCommon::EthernetMac& sourceAddress,
                                     const std::vector<uint8_t>& payload)
    {
        const uint16_t etherType = 0x0000; // no protocol
        std::vector<uint8_t> raw;
        std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(raw));
        std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(raw));
        auto etherTypeBytes = reinterpret_cast<const uint8_t*>(&etherType);
        raw.push_back(etherTypeBytes[1]); // We assume our platform to be little-endian
        raw.push_back(etherTypeBytes[0]);
        std::copy(payload.begin(), payload.end(), std::back_inserter(raw));
        return raw;
    }

    void SendFrame()
    {
        EthernetDemoCommon::EthernetMac WriterMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
        EthernetDemoCommon::EthernetMac BroadcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        _frameId++;

        std::stringstream stream;
        // Ensure that the payload is long enough to constitute a valid Ethernet frame
        stream << "Hello from Ethernet writer! (frameId=" << _frameId
               << ")----------------------------------------------------";
        auto payloadString = stream.str();
        std::vector<uint8_t> payload(payloadString.begin(), payloadString.end());
        auto frame = CreateFrame(BroadcastMacAddr, WriterMacAddr, payload);
        const auto userContext = reinterpret_cast<void*>(static_cast<intptr_t>(_frameId));

        std::stringstream ss;
        ss << "Sending Ethernet frame, data=" << EthernetDemoCommon::PrintPayload(payload, _printHex);
        GetLogger()->Info(ss.str());

        _ethernetController->SendFrame(EthernetFrame{frame}, userContext);
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
    args.participantName = "EthernetWriter";
    EthernetWriter app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Ethernet: Broadcast test frames");

    return app.Run();
}
