// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "ConsoleUI.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace std::chrono_literals;
using namespace SilKit::Services::Can;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Rpc;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    out << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << "ms";
    return out;
}

class InteractiveSUT : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IDataPublisher* _cyclicPublisher{nullptr};
    IDataPublisher* _reactivePublisher{nullptr};
    IDataSubscriber* _subscriber{nullptr};

    IRpcServer* _rpcServer{nullptr};
    IRpcClient* _rpcClient{nullptr};

    ICanController* _canController{nullptr};

    std::chrono::nanoseconds _asyncNow{0ns};

    bool _sendCan{false};
    bool _sendPubSub{false};
    bool _sendRpc{false};

    bool _reactiveCanMsg{false};
    bool _reactivePublishMsg{false};

    ConsoleUI _consoleUI;

    void AddCommandLineArgs() override
    {
    }

    void EvaluateCommandLineArgs() override
    {
    }

    void CreateControllers() override
    {
        // -----------------------------------
        // PubSub
        // -----------------------------------

        std::string pubSubMediaType{SilKit::Util::SerDes::MediaTypeData()};
        SilKit::Services::PubSub::PubSubSpec pubSpec{"ToCANoe", pubSubMediaType};
        pubSpec.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Optional);
        pubSpec.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Optional);
        pubSpec.AddLabel("Instance", "PubSubObject1", SilKit::Services::MatchingLabel::Kind::Optional);
        _cyclicPublisher = GetParticipant()->CreateDataPublisher("cyclicPublisher", pubSpec);

        SilKit::Services::PubSub::PubSubSpec pubSpecReactiveSend{"ToCANoe", pubSubMediaType};
        pubSpecReactiveSend.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Optional);
        pubSpecReactiveSend.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Optional);
        pubSpecReactiveSend.AddLabel("Instance", "ReactivePubSubObject1",
                                     SilKit::Services::MatchingLabel::Kind::Optional);
        _reactivePublisher = GetParticipant()->CreateDataPublisher("reactivePublisher", pubSpecReactiveSend);

        SilKit::Services::PubSub::PubSubSpec subSpec{"FromCANoe", pubSubMediaType};
        subSpec.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Mandatory);
        subSpec.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Mandatory);
        subSpec.AddLabel("Instance", "PubSubObject1", SilKit::Services::MatchingLabel::Kind::Mandatory);
        _subscriber = GetParticipant()->CreateDataSubscriber(
            "subscriber1", subSpec, [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);
            SilKit::Util::SerDes::Deserializer deserializer(eventData);
            int64_t t_send = deserializer.Deserialize<int64_t>(64);

            std::stringstream ss;
            ss << std::left << std::setw(8) << "RX data" << std::left << std::setw(10)
                   << " T_RX=" << dataMessageEvent.timestamp << std::setw(10) << "T_TX=" << t_send;
            GetLogger()->Info(ss.str());
            _consoleUI.Log(ss.str());

            if (_reactivePublishMsg)
            {
                PublishTimeNow(_reactivePublisher);
            }
        });

        // -----------------------------------
        // RPC
        // -----------------------------------

        std::string rpcMediaType{SilKit::Util::SerDes::MediaTypeRpc()};
        RpcSpec rpcSpecCallFromCANoe{"CallFromCANoe", rpcMediaType};
        rpcSpecCallFromCANoe.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Optional);
        rpcSpecCallFromCANoe.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Optional);
        // "Instance" Label Needed?
        rpcSpecCallFromCANoe.AddLabel("Instance", "RpcObject1", SilKit::Services::MatchingLabel::Kind::Optional);

        _rpcServer = GetParticipant()->CreateRpcServer("CallFromCANoe", rpcSpecCallFromCANoe,
                                                       [this](IRpcServer* server, const RpcCallEvent& event) {
            auto argumentDataVector = SilKit::Util::ToStdVector(event.argumentData);
            SilKit::Util::SerDes::Deserializer deserializer(argumentDataVector);
            const uint8_t argumentData = deserializer.Deserialize<uint8_t>(8);
            uint8_t resultData = argumentData + 100;

            std::stringstream ss;
            ss << "RPC from CANoe with argumentData=" << static_cast<unsigned int>(argumentData)
               << ", returning resultData=" << static_cast<unsigned int>(resultData);
            GetParticipant()->GetLogger()->Info(ss.str());
            _consoleUI.Log(ss.str());

            SilKit::Util::SerDes::Serializer serializer;
            serializer.Serialize<uint8_t>(resultData, 8);
            server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
        });

        RpcSpec rpcSpecCallToCANoe{"CallToCANoe", rpcMediaType};
        rpcSpecCallToCANoe.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Optional);
        rpcSpecCallToCANoe.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Optional);
        rpcSpecCallToCANoe.AddLabel("Instance", "RpcObject1", SilKit::Services::MatchingLabel::Kind::Optional);

        _rpcClient = GetParticipant()->CreateRpcClient("CallToCANoe", rpcSpecCallToCANoe,
                                                       [this](IRpcClient*, const RpcCallResultEvent& event) {
            auto resultDataVector = SilKit::Util::ToStdVector(event.resultData);
            uint8_t resultData = 0;
            if (!resultDataVector.empty())
            {
                SilKit::Util::SerDes::Deserializer deserializer(resultDataVector);
                resultData = deserializer.Deserialize<uint8_t>(8);
            }

            std::stringstream ss;
            switch (event.callStatus)
            {
            case RpcCallStatus::Success:
                ss << "RPC return from CANoe with resultData=" << resultData;
                break;
            case RpcCallStatus::ServerNotReachable:
                ss << "Warning: Call failed with RpcCallStatus::ServerNotReachable";
                break;
            case RpcCallStatus::UndefinedError:
                ss << "Warning: Call failed with RpcCallStatus::UndefinedError";
                break;
            case RpcCallStatus::InternalServerError:
                ss << "Warning: Call failed with RpcCallStatus::InternalServerError";
                break;
            case RpcCallStatus::Timeout:
                ss << "Warning: Call failed with RpcCallStatus::Timeout";
                break;
            }

            GetParticipant()->GetLogger()->Info(ss.str());
            _consoleUI.Log(ss.str());
        });


        // -----------------------------------
        // CAN
        // -----------------------------------

        _canController = GetParticipant()->CreateCanController("CAN1", "CAN1");
        _canController->AddFrameTransmitHandler([this](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
            // NOP
        });
        _canController->AddFrameHandler([this](ICanController* /*ctrl*/, const CanFrameEvent& frameEvent) {
            std::stringstream ss;
            ss << std::left << std::setw(8) << "RX CAN" << std::left << std::setw(10)
                   << "T=" << frameEvent.timestamp;
            GetLogger()->Info(ss.str());
            _consoleUI.Log(ss.str());

            if (_reactiveCanMsg)
            {
                SendFrame();
            }
        });
    }

    void InitControllers() override
    {
        _canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
        _canController->Start();
    }


    void SendFrame()
    {
        CanFrame canFrame{};
        canFrame.canId = 1;
        static int msgId = 0;
        const auto currentMessageId = msgId++;

        std::stringstream payloadBuilder;
        payloadBuilder << "CAN " << (currentMessageId % 100);
        auto payloadStr = payloadBuilder.str();
        std::vector<uint8_t> payloadBytes(payloadStr.begin(), payloadStr.end());
        canFrame.dataField = payloadBytes;
        canFrame.dlc = static_cast<uint16_t>(canFrame.dataField.size());

        std::stringstream ss;
        ss << std::left << std::setw(8) << "TX CAN" << std::left << std::setw(10) << "T=" << GetTimeNow();
        GetLogger()->Info(ss.str());
        _consoleUI.Log(ss.str());

        _canController->SendFrame(std::move(canFrame));
    }

    void PublishTimeNow(IDataPublisher* publisher)
    {
        auto now = GetTimeNow();

        std::stringstream ss;
        ss << std::left << std::setw(8) << "TX Data " << std::left << std::setw(10) << "T=" << now;
        GetLogger()->Info(ss.str());
        _consoleUI.Log(ss.str());

        SilKit::Util::SerDes::Serializer ser;
        int64_t nowNs = now.count();
        ser.Serialize(nowNs, 64);
        publisher->Publish(ser.ReleaseBuffer());
    }


    void RPC()
    {
        auto rndData = static_cast<uint8_t>(rand() % 10);
        std::vector<uint8_t> argumentData{rndData};

        SilKit::Util::SerDes::Serializer serializer;
        serializer.Serialize(argumentData);

        _rpcClient->Call(serializer.ReleaseBuffer(), nullptr);
        std::stringstream ss;
        ss << "RPC with argumentData=" << static_cast<unsigned int>(rndData) << std::endl;
        GetParticipant()->GetLogger()->Info(ss.str());
        _consoleUI.Log(ss.str());
    }
    

    auto GetTimeNow() -> std::chrono::nanoseconds
    {
        if (GetArguments().runAsync)
        {
            return _asyncNow;
        }
        else
        {
            return GetTimeSyncService()->Now();
        }
    }


    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        std::stringstream ss;
        ss << std::left << std::setw(8) << "SimStep" << std::left << std::setw(10) << "T=" << now;
        GetLogger()->Info(ss.str());
        _consoleUI.Log(ss.str());

        DoWork();
    }

    void DoWorkAsync() override
    {
        DoWork();
        _asyncNow += GetArguments().duration;
    }

    void DoWork()
    {
        if (_sendCan)
        {
            SendFrame();
        }
        if (_sendPubSub)
        {
            PublishTimeNow(_cyclicPublisher);
        }
        if (_sendRpc)
        {
            RPC();
        }
    }

public:
    
    void SetupUI()
    { 
        _consoleUI.SetupScreen();

        _consoleUI.AddButton("CAN", [this]() {
            SendFrame();
        });

        _consoleUI.AddButton("Publish", [this]() {
            PublishTimeNow(_cyclicPublisher);
        });

        _consoleUI.AddButton("RPC", [this]() {
            RPC();
        });

        _consoleUI.AddTimerCheckbox("Cyclic CAN", _sendCan);
        _consoleUI.AddTimerCheckbox("Cyclic PubSub", _sendPubSub);
        _consoleUI.AddTimerCheckbox("Cyclic RPC", _sendRpc);

        _consoleUI.AddReactiveCheckbox("Reactive CAN", _reactiveCanMsg);
        _consoleUI.AddReactiveCheckbox("Reactive PubSub", _reactivePublishMsg);
    }

    void StartRenderThread()
    { 
        _consoleUI.StartRenderThread();
    }

       
    void StopRenderThread() 
    { 
        _consoleUI.StopRenderThread(); 
    }

};

int main(int argc, char** argv)
{
    constexpr bool interactiveUI = true;
    std::string appDescription = "Counterpart of the CANoe time sync. test configuration";

    Arguments args;
    args.duration = 10ms;
    args.participantName = "P1";
    InteractiveSUT app{args};

    if (interactiveUI)
    {
        app.SetupUI();
        app.SetupCommandLineArgs(argc, argv, appDescription,
                                 {ApplicationBase::DefaultArg::Async, ApplicationBase::DefaultArg::Log});
        app.StartRenderThread();
        auto appReturnCode = app.Run();
        app.StopRenderThread();
        return appReturnCode;
    }
    else
    {
        app.SetupCommandLineArgs(argc, argv, appDescription, {ApplicationBase::DefaultArg::Async});
        return app.Run();
    }
}

