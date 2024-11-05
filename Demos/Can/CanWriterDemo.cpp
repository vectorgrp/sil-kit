#include <sstream>

#include "../Common/DemoRunner.hpp"

#include "silkit/Silkit.hpp"
#include "silkit/services/can/all.hpp"
#include "silkit/services/can/string_utils.hpp"

using namespace SilKit::Services::Can;

void FrameTransmitHandler(const CanFrameTransmitEvent& ack, ILogger* logger)
{
    std::stringstream buffer;
    buffer << ">> " << ack.status << " for CAN frame with timestamp=" << ack.timestamp
           << " and userContext=" << ack.userContext;
    logger->Info(buffer.str());
}

void FrameHandler(const CanFrameEvent& frameEvent, ILogger* logger)
{
    std::string payload(frameEvent.frame.dataField.begin(), frameEvent.frame.dataField.end());
    std::stringstream buffer;
    buffer << ">> CAN frame: canId=" << frameEvent.frame.canId << " timestamp=" << frameEvent.timestamp << " \""
           << payload << "\"";
    logger->Info(buffer.str());
}


void SendFrame(ICanController* controller, ILogger* logger)
{
    CanFrame canFrame{};
    canFrame.canId = 3;
    canFrame.flags |= static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf) // FD Format Indicator
                      | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs); // Bit Rate Switch (for FD Format only)

    static int msgId = 0;
    const auto currentMessageId = msgId++;

    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << (currentMessageId % 100);
    auto payloadStr = payloadBuilder.str();

    std::vector<uint8_t> payloadBytes;
    payloadBytes.resize(payloadStr.size());
    std::copy(payloadStr.begin(), payloadStr.end(), payloadBytes.begin());

    canFrame.dataField = payloadBytes;
    canFrame.dlc = static_cast<uint16_t>(canFrame.dataField.size());

    void* const userContext = reinterpret_cast<void*>(static_cast<intptr_t>(currentMessageId));

    controller->SendFrame(std::move(canFrame), userContext);
    std::stringstream buffer;
    buffer << "<< CAN frame sent with userContext=" << userContext;
    logger->Info(buffer.str());
}

struct CanWriterDemo : public SilKitDemo::IDemo
{
   ICanController* canController;

    void CreateControllers(SilKitDemo::Context &context) override
    {
        canController = context.participant->CreateCanController("CAN1", "CAN1");
        canController->AddFrameTransmitHandler(
            [context](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
            FrameTransmitHandler(ack, context.logger);
        });
        canController->AddFrameHandler([context](ICanController* /*ctrl*/, const CanFrameEvent& frameEvent) {
            FrameHandler(frameEvent, context.logger);
        });

    }

    void InitControllers(SilKitDemo::Context& context) override
    {
        canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
        canController->Start();
    }

    void DoWork(SilKitDemo::Context &context) override
    {
        SendFrame(canController, context.logger);
    }

    void Teardown(SilKitDemo::Context &context) override
    {

    }
};

int main(int argc, char **argv)
{
    CanWriterDemo canWriterDemo;
    return SilKitDemo::Run("CanWriter", canWriterDemo, argc, argv);
}
