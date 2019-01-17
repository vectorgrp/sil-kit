// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::util;
using namespace ib::sim;

using namespace std::chrono_literals;
using namespace std::placeholders;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

std::ostream& operator<<(std::ostream& out, fr::PocState state)
{
    switch (state)
    {
    case  fr::PocState::DefaultConfig:
        out << "DefaultConfig";
        break;
    case  fr::PocState::Config:
        out << "Config";
        break;
    case  fr::PocState::Ready:
        out << "Ready";
        break;
    case  fr::PocState::Startup:
        out << "Startup";
        break;
    case  fr::PocState::Wakeup:
        out << "Wakeup";
        break;
    case  fr::PocState::NormalActive:
        out << "NormalActive";
        break;
    case  fr::PocState::NormalPassive:
        out << "NormalPassive";
        break;
    case  fr::PocState::Halt:
        out << "Halt";
        break;
    default:
        out << "state=" << static_cast<uint32_t>(state);
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, fr::Channel channel)
{
    switch (channel)
    {
    case  fr::Channel::None:
        out << "None";
        break;
    case  fr::Channel::A:
        out << "A";
        break;
    case  fr::Channel::B:
        out << "B";
        break;
    case  fr::Channel::AB:
        out << "AB";
        break;
    default:
        out << "channel=" << static_cast<uint32_t>(channel);
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, fr::SymbolPattern symbolPattern)
{
    switch (symbolPattern)
    {
    case fr::SymbolPattern::CasMts:
        out << "CasMts";
        break;
    case fr::SymbolPattern::Wus:
        out << "Wus";
        break;
    case fr::SymbolPattern::Wudop:
        out << "Wudop";
        break;
    default:
        out << "pattern=" << static_cast<uint8_t>(symbolPattern);
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const fr::Header& header)
{
    out << "Header{f=["
        << (header.IsSet(fr::Header::Flag::SuFIndicator) ? "U" : "-")
        << (header.IsSet(fr::Header::Flag::SyFIndicator) ? "Y" : "-")
        << (header.IsSet(fr::Header::Flag::NFIndicator) ? "-" : "N")
        << (header.IsSet(fr::Header::Flag::PPIndicator) ? "P" : "-")
        << "],s=" << header.frameId
        << ",l=" << (uint32_t)header.payloadLength
        << ",crc=" << std::hex << header.headerCrc << std::dec
        << ",c=" << (uint32_t)header.cycleCount
        << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const fr::FrMessage& msg)
{
    out << "FrMessage:    channel=" << msg.channel
        << " " << msg.frame.header;

    if (msg.frame.header.IsSet(fr::Header::Flag::NFIndicator))
    {
        std::string payloadString(msg.frame.payload.begin(), msg.frame.payload.end());
        out << " payload=\"" << payloadString << "\"";
    }

    std::chrono::nanoseconds ignoreTime;
    if (msg.timestamp != ignoreTime)
        out << " @t=" << msg.timestamp;

    return out;
}

std::ostream& operator<<(std::ostream& out, const fr::FrMessageAck& msg)
{
    out << "FrMessageAck: channel=" << msg.channel
        << " " << msg.frame.header;

    if (msg.frame.header.IsSet(fr::Header::Flag::NFIndicator))
    {
        std::string payloadString(msg.frame.payload.begin(), msg.frame.payload.end());
        out << " payload=\"" << payloadString << "\"";
    }

    std::chrono::nanoseconds ignoreTime;
    if (msg.timestamp != ignoreTime)
        out << " @t=" << msg.timestamp;

    out << " txBufferId=" << msg.txBufferIndex;
    return out;
}

std::ostream& operator<<(std::ostream& out, const fr::FrSymbol& symbol)
{
    out << "FrSymbol{t=" << symbol.timestamp
        << ", channel=" << symbol.channel
        << ", pattern=" << symbol.pattern
        << "}";

    return out;
}

std::ostream& operator<<(std::ostream& out, const fr::FrSymbolAck& symbol)
{
    out << "FrSymbolAck{t=" << symbol.timestamp
        << ", channel=" << symbol.channel
        << ", pattern=" << symbol.pattern
        << "}";

    return out;
}


template<typename T>
void ReceiveMessage(fr::IFrController* /*controller*/, const T& t)
{
    std::cout << ">> " << t << std::endl;
}



struct FlexRayUser
{
    FlexRayUser(fr::IFrController* controller)
        : controller{controller}
    {
    }

    void configure(fr::ControllerConfig&& config)
    {
        controllerConfig = std::move(config);

        controller->Configure(controllerConfig);
        
    }

    void doAction(std::chrono::nanoseconds now)
    {
        switch (state)
        {
        case fr::PocState::Ready:
            pocReady(now);
            break;
        case fr::PocState::NormalActive:
            txBufferUpdate(now);
            break;
        case fr::PocState::DefaultConfig:
        case fr::PocState::Config:
        case fr::PocState::Startup:
        case fr::PocState::Wakeup:
        case fr::PocState::NormalPassive:
        case fr::PocState::Halt:
            return;
        }
    }

    void pocReady(std::chrono::nanoseconds /*now*/)
    {
        switch (busState)
        {
        case MasterState::PerformWakeup:
            controller->Wakeup();
            return;
        case MasterState::WaitForWakeup:
            return;
        case MasterState::WakeupDone:
            controller->AllowColdstart();
            controller->Run();
            return;
        default:
            return;
        }
    }

    void txBufferUpdate(std::chrono::nanoseconds /*now*/)
    {
        if (controllerConfig.bufferConfigs.empty())
            return;

        static auto msgNumber = -1;
        msgNumber++;

        auto bufferIdx = msgNumber % controllerConfig.bufferConfigs.size();

        // prepare a friendly message as payload
        std::stringstream payloadStream;
        payloadStream << "FrMessage#" << msgNumber
                      << " sent from buffer " << bufferIdx;
        auto payloadString = payloadStream.str();


        fr::TxBufferUpdate update;
        update.payload.resize(payloadString.size());
        update.payloadDataValid = true;
        update.txBufferIndex = static_cast<decltype(update.txBufferIndex)>(bufferIdx);

        std::copy(payloadString.begin(), payloadString.end(), update.payload.begin());
        //update.payload[payloadString.size()] = 0;

        controller->UpdateTxBuffer(update);
    }

    void ControllerStatusHandler(fr::IFrController* /*controller*/, const fr::ControllerStatus& status)
    {
        std::cout << ">> POC=" << status.pocState
                  << " @t=" << status.timestamp
                  << std::endl;

        auto oldState = state;
        state = status.pocState;

        if (oldState == fr::PocState::Wakeup
            && state == fr::PocState::Ready)
        {
            std::cout << "   Wakeup finished..." << std::endl;
            busState = MasterState::WakeupDone;
        }

    }

    void WakeupHandler(fr::IFrController* controller, const fr::FrSymbol& symbol)
    {
        std::cout << ">> WAKEUP! (" << symbol.pattern << ")" << std::endl;
        controller->AllowColdstart();
        controller->Run();
    }


    fr::IFrController* controller = nullptr;

    fr::ControllerConfig controllerConfig;
    fr::PocState state = fr::PocState::DefaultConfig;

    enum class MasterState
    {
        Ignore,
        PerformWakeup,
        WaitForWakeup,
        WakeupDone
    };
    MasterState busState = MasterState::Ignore;
};

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }


    ib::cfg::Config ibConfig;
    try
    {
        ibConfig = ib::cfg::Config::FromJsonFile(argv[1]);
    }
    catch (const ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << (&error)->what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }


    std::string participantName(argv[2]);


    uint32_t domainId = 42;
    if (argc >= 4)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[3]));
    }


    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);
    auto controller = comAdapter->CreateFlexrayController("FlexRay1");


    // Set an Init Handler
    auto&& participantController = comAdapter->GetParticipantController();
    participantController->SetInitHandler([&participantName](auto initCmd) {

        std::cout << "Initializing " << participantName << std::endl;

    });

    // Set a Stop Handler
    participantController->SetStopHandler([]() {

        std::cout << "Stopping..." << std::endl;

    });

    // Set a Shutdown Handler
    participantController->SetShutdownHandler([]() {

        std::cout << "Shutting down..." << std::endl;

    });

    // Setup Cluster Parameters
    fr::ClusterParameters clusterParams;
    {
        clusterParams.gColdstartAttempts = 8;
        clusterParams.gCycleCountMax = 63;
        clusterParams.gdActionPointOffset = 2;
        clusterParams.gdDynamicSlotIdlePhase = 1;
        clusterParams.gdMiniSlot = 5;
        clusterParams.gdMiniSlotActionPointOffset = 2;
        clusterParams.gdStaticSlot = 31;
        clusterParams.gdSymbolWindow = 1;
        clusterParams.gdSymbolWindowActionPointOffset = 1;
        clusterParams.gdTSSTransmitter = 9;
        clusterParams.gdWakeupTxActive = 60;
        clusterParams.gdWakeupTxIdle = 180;
        clusterParams.gListenNoise = 2;
        clusterParams.gMacroPerCycle = 3636;
        clusterParams.gMaxWithoutClockCorrectionFatal = 2;
        clusterParams.gMaxWithoutClockCorrectionPassive = 2;
        clusterParams.gNumberOfMiniSlots = 291;
        clusterParams.gNumberOfStaticSlots = 70;
        clusterParams.gPayloadLengthStatic = 16;
        clusterParams.gSyncFrameIDCountMax = 15;
    }
    fr::NodeParameters nodeParams;
    {
        nodeParams.pAllowHaltDueToClock = 1;
        nodeParams.pAllowPassiveToActive = 0;
        nodeParams.pChannels = fr::Channel::AB;
        nodeParams.pClusterDriftDamping = 2;
        nodeParams.pdAcceptedStartupRange = 212;
        nodeParams.pdListenTimeout = 400162;
        nodeParams.pKeySlotId = 0;
        nodeParams.pKeySlotOnlyEnabled = 0;
        nodeParams.pKeySlotUsedForStartup = 0;
        nodeParams.pKeySlotUsedForSync = 0;
        nodeParams.pLatestTx = 249;
        nodeParams.pMacroInitialOffsetA = 3;
        nodeParams.pMacroInitialOffsetB = 3;
        nodeParams.pMicroInitialOffsetA = 6;
        nodeParams.pMicroInitialOffsetB = 6;
        nodeParams.pMicroPerCycle = 200000;
        nodeParams.pOffsetCorrectionOut = 127;
        nodeParams.pOffsetCorrectionStart = 3632;
        nodeParams.pRateCorrectionOut = 81;
        nodeParams.pWakeupChannel = fr::Channel::A;
        nodeParams.pWakeupPattern = 33;

        nodeParams.pdMicrotick = fr::ClockPeriod::T25NS;
        nodeParams.pSamplesPerMicrotick = 2;
    }

    std::vector<fr::TxBufferConfig> bufferConfigs;

    if (participantName == "Node0")
    {
        // initialize bufferConfig to send some FrMessages
        fr::TxBufferConfig cfg;
        cfg.channels = fr::Channel::A;
        cfg.slotId = 10;
        cfg.offset = 0;
        cfg.repetition = 1;
        cfg.hasPayloadPreambleIndicator = false;
        cfg.headerCrc = 5;
        cfg.transmissionMode = fr::TransmissionMode::SingleShot;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::B;
        cfg.slotId = 20;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::AB;
        cfg.slotId = 30;
        bufferConfigs.push_back(cfg);

        // this controller performs the wakeup -> no key slots used
        nodeParams.pKeySlotId = 10;
        nodeParams.pKeySlotUsedForStartup = 1;
    }
    else if (participantName == "Node1")
    {
        // initialize bufferConfig to send some FrMessages
        fr::TxBufferConfig cfg;
        cfg.channels = fr::Channel::A;
        cfg.slotId = 11;
        cfg.offset = 0;
        cfg.repetition = 1;
        cfg.hasPayloadPreambleIndicator = false;
        cfg.headerCrc = 5;
        cfg.transmissionMode = fr::TransmissionMode::SingleShot;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::B;
        cfg.slotId = 21;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::AB;
        cfg.slotId = 31;
        bufferConfigs.push_back(cfg);

        // this controller performs the wakeup -> no key slots used
        nodeParams.pKeySlotId = 11;
        nodeParams.pKeySlotUsedForStartup = 1;
    }
    else if (participantName == "Node2")
    {
        // initialize bufferConfig to send some FrMessages
        fr::TxBufferConfig cfg;
        cfg.channels = fr::Channel::A;
        cfg.slotId = 12;
        cfg.offset = 0;
        cfg.repetition = 1;
        cfg.hasPayloadPreambleIndicator = false;
        cfg.headerCrc = 5;
        cfg.transmissionMode = fr::TransmissionMode::SingleShot;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::B;
        cfg.slotId = 22;
        bufferConfigs.push_back(cfg);

        cfg.channels = fr::Channel::AB;
        cfg.slotId = 32;
        bufferConfigs.push_back(cfg);

        // this controller performs the wakeup -> no key slots used
        nodeParams.pKeySlotId = 12;
        nodeParams.pKeySlotUsedForStartup = 1;
    }

    FlexRayUser frUser(controller);
    if (participantName == "Node0")
        frUser.busState = FlexRayUser::MasterState::PerformWakeup;

    controller->RegisterControllerStatusHandler(bind_method(&frUser, &FlexRayUser::ControllerStatusHandler));
    controller->RegisterMessageHandler(&ReceiveMessage<fr::FrMessage>);
    controller->RegisterMessageAckHandler(&ReceiveMessage<fr::FrMessageAck>);
    controller->RegisterWakeupHandler(bind_method(&frUser, &FlexRayUser::WakeupHandler));
    controller->RegisterSymbolHandler(&ReceiveMessage<fr::FrSymbol>);
    controller->RegisterSymbolAckHandler(&ReceiveMessage<fr::FrSymbolAck>);

    fr::ControllerConfig config;
    config.clusterParams = clusterParams;
    config.nodeParams = nodeParams;
    config.bufferConfigs = bufferConfigs;

    frUser.configure(std::move(config));

    participantController->SetSimulationTask(
        [&frUser](std::chrono::nanoseconds now)
        {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            std::cout << "now=" << nowMs.count() << "ms" << std::endl;
            frUser.doAction(now);
            std::this_thread::sleep_for(1s);
        }
    );

    //auto finalStateFuture = participantController->RunAsync();
    //auto finalState = finalStateFuture.get();

    auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}
