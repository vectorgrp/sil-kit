// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/functional.hpp"

using namespace ib::mw;
using namespace ib::sim;

using namespace std::chrono_literals;
using namespace std::placeholders;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void ReceiveMessage(lin::ILinController* /*controller*/, const lin::LinMessage& msg)
{
    std::string msgString(msg.payload.size, 0);
    std::copy(msg.payload.data.begin(), msg.payload.data.begin() + msg.payload.size, msgString.begin());

    std::cout << ">> LIN Message: linId=" << static_cast<unsigned int>(msg.linId)
              << " timestamp=" << msg.timestamp
              << " \"" << msgString << "\""
              << std::endl;
}

struct LinMaster
{
    void doAction(std::chrono::nanoseconds now)
    {
        switch (state)
        {
        case State::SendMessage:
            if (now > gotoSleepTime)
                GotoSleep();
            else
                SendMessage();
            return;
        case State::WaitForAck:
            return;
        case State::RequestMessage:
            RequestMessage();
            return;
        case State::WaitForReply:
            return;
        }
    }
    void SendMessage()
    {
        lin::LinMessage msg;
        msg.linId = 17;
        msg.checksumModel = lin::ChecksumModel::Undefined;

        static int msgId = 0;
        std::stringstream stream;
        stream << "LIN " << std::setw(3) << msgId++;

        auto hello = stream.str();
        msg.payload.size = std::min<uint8_t>(8u, static_cast<uint8_t>(hello.size()));
        msg.payload.data.fill(0);
        std::copy(hello.begin(), hello.begin() + msg.payload.size, msg.payload.data.begin());

        state = State::WaitForAck;
        controller->SendMessage(msg);
        std::cout << "<< LIN message sent with linId=" << static_cast<unsigned int>(msg.linId) << std::endl;
    }

    void RequestMessage()
    {
        lin::RxRequest request;
        request.linId = 34;
        request.checksumModel = lin::ChecksumModel::Enhanced;
        request.payloadLength = 6;

        state = State::WaitForReply;
        controller->RequestMessage(request);
        std::cout << "<< LIN REQUEST sent for linId=" << static_cast<unsigned int>(request.linId) << std::endl;
    }

    void ReceiveReply(lin::ILinController* controller, const lin::LinMessage& msg)
    {
        ReceiveMessage(controller, msg);
        std::cout << "    status=" << msg.status << std::endl;
        if (state == State::WaitForReply)
            state = State::SendMessage;
    }

    void ReceiveTxComplete(lin::ILinController* /*controller*/, lin::MessageStatus status)
    {
        std::cout << ">> TX Notification: status=" << status << std::endl;
        switch (state)
        {
        case State::WaitForAck:
            state = State::RequestMessage;
            break;

        case State::GotoSleep:
            state = State::Sleeping;
            controller->SetSleepMode();
            break;

        default:
            break;
        }
    }

    void GotoSleep()
    {
        state = State::GotoSleep;
        gotoSleepTime = std::chrono::nanoseconds::max();
        std::cout << "<< Sending Go-To-Sleep Command." << std::endl;
        controller->SendGoToSleep();
    }

    void WakeupRequest(lin::ILinController* controller)
    {

        switch (state)
        {
        case State::GotoSleep:
            std::cerr << "WARNING: Received Wakeup Request in state GotoSleep. Check if the Gotosleep was not acknowledged!\n" << std::endl;
            break;
        case State::Sleeping:
            std::cout << ">> Wakeup Request received" << std::endl;
            break;
        default:
            std::cerr << "ERROR: Received Wakeup Request while not sleeping\n" << std::endl;
            return;
        }

        state = State::SendMessage;
        controller->SetOperationalMode();
    }

    enum class State
    {
        SendMessage,
        WaitForAck,
        RequestMessage,
        WaitForReply,
        GotoSleep,
        Sleeping
    };

    lin::ILinController* controller = nullptr;
    State state = State::SendMessage;
    std::chrono::nanoseconds gotoSleepTime{std::chrono::nanoseconds::max()};

};

struct LinSlave
{
    void DoAction(std::chrono::nanoseconds now_)
    {
        now = now_;

        switch (state)
        {
        case State::Sleeping:
            CheckWakeup();
            break;

        default:
            break;
        }
    }


    void CheckWakeup()
    {
        if (now < wakeupTime)
            return;


        std::cout << "LIN Slave sending LIN Wakupe Request" << std::endl;
        state = State::PendingWakup;
        linController->SendWakeupRequest();
    }


    void SleepCommandHandler(lin::ILinController* linController)
    {
        std::cout << "LIN Slave Received Sleep Command." << std::endl;
        std::cout << "LIN Slave is entering sleep mode." << std::endl;
        state = State::Sleeping;
        wakeupTime = now + 20ms;
        linController->SetSleepMode();
    }

    void WakeupRequestHandler(lin::ILinController* linController)
    {
        std::cout << "LIN Slave Received Wakeup Request..." << std::endl;
        state = LinSlave::State::Operational;
        linController->SetOperationalMode();
    }


    enum class State
    {
        Invalid,
        Operational,
        Sleeping,
        PendingWakup
    };

    State state{State::Operational};

    lin::ILinController* linController{nullptr};
    std::chrono::nanoseconds now{0ns};
    std::chrono::nanoseconds wakeupTime{0ns};

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
    auto* linController = comAdapter->CreateLinController("LIN1");

    // Create a SyncMaster if the participant is configured to do so.
    // NB: New SyncMaster Interface is subject to changes (cf. AFTMAGT-124)
    auto& participantConfig = ib::cfg::get_by_name(ibConfig.simulationSetup.participants, participantName);
    if (participantConfig.isSyncMaster)
    {
        comAdapter->CreateSyncMaster();
        std::cout << "Created SyncMaster at Participant: " << participantName << std::endl;
    }


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

    participantController->SetPeriod(1ms);

    LinMaster master;
    LinSlave slave;

    if (participantName == "LinMaster")
    {
        master.controller = linController;
        master.gotoSleepTime = 10ms;

        linController->SetMasterMode();
        linController->SetBaudRate(20'000);
        linController->RegisterTxCompleteHandler(std::bind(&LinMaster::ReceiveTxComplete, &master, _1, _2));
        linController->RegisterReceiveMessageHandler(std::bind(&LinMaster::ReceiveReply, &master, _1, _2));
        linController->RegisterWakeupRequestHandler(ib::util::bind_method(&master, &LinMaster::WakeupRequest));

        participantController->SetSimulationTask(
            [&master](std::chrono::nanoseconds now)
            {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                master.doAction(now);
            }
        );
    }
    else
    {
        slave.linController = linController;

        linController->SetSlaveMode();
        linController->SetBaudRate(20'000);
        linController->RegisterReceiveMessageHandler(ReceiveMessage);

        lin::SlaveConfiguration slaveConfig;

        // Configure LIN Controller to trigger a callback on LIN ID 17
        lin::SlaveResponseConfig response;
        response.linId = 17;
        response.responseMode = lin::ResponseMode::Rx;
        response.checksumModel = lin::ChecksumModel::Enhanced;
        response.payloadLength = 8;
        slaveConfig.responseConfigs.push_back(response);

        // Configure LIN Controller to send a reply on LIN ID 34
        response.linId = 34;
        response.responseMode = lin::ResponseMode::TxUnconditional;
        response.checksumModel = lin::ChecksumModel::Enhanced;
        response.payloadLength = 6;
        slaveConfig.responseConfigs.push_back(response);

        linController->SetSlaveConfiguration(slaveConfig);


        std::string replyString= "HELLO!";

        lin::Payload replyPayload;
        replyPayload.size = static_cast<uint8_t>(replyString.length());
        std::copy(replyString.begin(), replyString.end(), replyPayload.data.begin());

        linController->SetResponse(34, replyPayload);

        linController->RegisterSleepCommandHandler(ib::util::bind_method(&slave, &LinSlave::SleepCommandHandler));
        linController->RegisterWakeupRequestHandler(ib::util::bind_method(&slave, &LinSlave::WakeupRequestHandler));

        participantController->SetSimulationTask([&slave, linController](std::chrono::nanoseconds now)
        {
            std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
            std::this_thread::sleep_for(500ms);

            slave.DoAction(now);

        });
    }


    auto finalStateFuture = participantController->RunAsync();
    auto finalState = finalStateFuture.get();

    //auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}