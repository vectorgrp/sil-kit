#include "../Common/DemoRunner.hpp"

#include "silkit/Silkit.hpp"

using namespace SilKit::Services::Ethernet;

// Field in a frame that can indicate the protocol, payload size, or the start of a VLAN tag
using EtherType = uint16_t;
using EthernetMac = std::array<uint8_t, 6>;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

std::vector<uint8_t> CreateFrame(const EthernetMac& destinationAddress, const EthernetMac& sourceAddress,
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

std::string GetPayloadStringFromFrame(const EthernetFrame& frame)
{
    const size_t FrameHeaderSize = 2 * sizeof(EthernetMac) + sizeof(EtherType);

    std::vector<uint8_t> payload;
    payload.insert(payload.end(), frame.raw.begin() + FrameHeaderSize, frame.raw.end());
    std::string payloadString(payload.begin(), payload.end());
    return payloadString;
}

void FrameTransmitHandler(IEthernetController* /*controller*/, const EthernetFrameTransmitEvent& frameTransmitEvent)
{
    if (frameTransmitEvent.status == EthernetTransmitStatus::Transmitted)
    {
        std::cout << ">> ACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext << std::endl;
    }
    else
    {
        std::cout << ">> NACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext;
        switch (frameTransmitEvent.status)
        {
        case EthernetTransmitStatus::Transmitted:
            break;
        case EthernetTransmitStatus::InvalidFrameFormat:
            std::cout << ": InvalidFrameFormat";
            break;
        case EthernetTransmitStatus::ControllerInactive:
            std::cout << ": ControllerInactive";
            break;
        case EthernetTransmitStatus::LinkDown:
            std::cout << ": LinkDown";
            break;
        case EthernetTransmitStatus::Dropped:
            std::cout << ": Dropped";
            break;
        }

        std::cout << std::endl;
    }
}

void FrameHandler(IEthernetController* /*controller*/, const EthernetFrameEvent& frameEvent)
{
    auto frame = frameEvent.frame;
    auto payload = GetPayloadStringFromFrame(frame);
    std::cout << ">> Ethernet frame: \"" << payload << "\"" << std::endl;
}

void SendFrame(IEthernetController* controller, const EthernetMac& from, const EthernetMac& to)
{
    static int frameId = 0;
    std::stringstream stream;
    stream
        << "Hello from Ethernet writer! (frameId =" << frameId++
        << ")"
           "----------------------------------------------------"; // ensure that the payload is long enough to constitute a valid Ethernet frame

    auto payloadString = stream.str();
    std::vector<uint8_t> payload(payloadString.size() + 1);
    memcpy(payload.data(), payloadString.c_str(), payloadString.size() + 1);

    const auto userContext = reinterpret_cast<void*>(static_cast<intptr_t>(frameId));

    auto frame = CreateFrame(to, from, payload);
    controller->SendFrame(EthernetFrame{frame}, userContext);
    std::cout << "<< ETH Frame sent with userContext=" << userContext << std::endl;
}

struct EthernetWriterDemo : public SilKitDemo::IDemo
{
    IEthernetController* ethernetController;
    const EthernetMac WriterMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
    const EthernetMac BroadcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    void CreateControllers(SilKitDemo::Context &context) override
    {
        ethernetController = context.participant->CreateEthernetController("Eth1", "Eth1");
        ethernetController->AddFrameHandler(&FrameHandler);
        ethernetController->AddFrameTransmitHandler(&FrameTransmitHandler);
    }

    void InitControllers(SilKitDemo::Context& context) override
    {
        ethernetController->Activate();
    }

    void DoWork(SilKitDemo::Context &context) override
    {
        SendFrame(ethernetController, WriterMacAddr, BroadcastMacAddr);
    }

    void Teardown(SilKitDemo::Context &context) override
    {
        // NOP
    }
};

int main(int argc, char **argv)
{
    EthernetWriterDemo ethernetWriterDemo;
    return Run("EthernetWriter", ethernetWriterDemo, argc, argv);
}
