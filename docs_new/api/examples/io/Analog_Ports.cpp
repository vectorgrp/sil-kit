// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"


#include <iostream>
#include <thread>
#include <chrono>

using namespace ib::cfg;
using namespace ib::sim::io;
using namespace std::chrono_literals;

const auto domainId = 123;
const std::string portName{"AIO"};

void writer(Config config)
{
    auto comAdapter = ib::CreateComAdapter(config, "AnalogWriter", domainId);
    auto* analogOut = comAdapter->CreateAnalogOut(portName);
    auto* participantCtrl = comAdapter->GetParticipantController();

    participantCtrl->SetPeriod(1ms);

    participantCtrl->SetSimulationTask(
        [analogOut, participantCtrl](std::chrono::nanoseconds now) {
            static auto maxUpdates = 10;

            double currentValue = analogOut->Read();
            currentValue += 0.13;
            analogOut->Write(currentValue,now);

            if(--maxUpdates < 1 ) { participantCtrl->Stop("Demo finished"); }
    });

    participantCtrl->Run();
}
void reader(Config config)
{
    auto comAdapter = ib::CreateComAdapter(config, "AnalogReader", domainId);
    auto* analogIn = comAdapter->CreateAnalogIn(portName);
    auto* participantCtrl = comAdapter->GetParticipantController();

    participantCtrl->SetPeriod(1ms);

    participantCtrl->SetSimulationTask(
        [analogIn, participantCtrl](std::chrono::nanoseconds now) {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

            double currentValue = analogIn->Read();

            std::cout <<nowMs <<"ms: SimulationTask current value is " << currentValue << std::endl;
    });

    //register a receive callback
    analogIn->RegisterHandler(
        [](IAnalogInPort* port, const double &value) {
        auto config = port->Config();
        std::cout << "Callback: port " << config.name
            <<" received new value " << value << std::endl;
    });
    participantCtrl->Run();
}
int main( int c, char** argv)
{
    auto config = ib::cfg::Config::FromJsonFile("Analog_Ports.json");

    std::thread writerThread{writer, config};
    std::thread readerThread{reader, config};

    if(writerThread.joinable()) writerThread.join();
    if(readerThread.joinable()) readerThread.join();
}
