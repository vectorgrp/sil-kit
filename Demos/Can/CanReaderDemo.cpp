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

struct CanReaderDemo : public SilKitDemo::IDemo
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
        // NOP
    }

    void Teardown(SilKitDemo::Context &context) override
    {
        // NOP
    }
};

int main(int argc, char **argv)
{
    CanReaderDemo canReaderDemo;
    return Run("CanReader", canReaderDemo, argc, argv);
}
