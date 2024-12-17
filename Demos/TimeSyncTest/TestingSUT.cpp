// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
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

class TestingSUT : public ApplicationBase
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

    std::chrono::nanoseconds _now{0ns};

    bool _sendCan{false};
    bool _sendPubSub{false};
    bool _sendRpc{false};

    void AddCommandLineArgs() override {}

    void EvaluateCommandLineArgs() override {}

    void CreateControllers() override
    {
        // -----------------------------------
        // PubSub
        // -----------------------------------

        std::string pubSubMediaType{SilKit::Util::SerDes::MediaTypeData()};

        SilKit::Services::PubSub::PubSubSpec subSpec{"TriggerCompleteSimStep", pubSubMediaType};
        subSpec.AddLabel("VirtualNetwork", "Default", SilKit::Services::MatchingLabel::Kind::Mandatory);
        subSpec.AddLabel("Namespace", "TimeSyncTest", SilKit::Services::MatchingLabel::Kind::Mandatory);
        subSpec.AddLabel("Instance", "PubSubObject1", SilKit::Services::MatchingLabel::Kind::Mandatory);
        _subscriber = GetParticipant()->CreateDataSubscriber(
            "subscriber1", subSpec, [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {

                std::stringstream ss;
                ss  << "Triggering CompleteSimulationStep via DO at T=" << _now;
                GetLogger()->Info(ss.str());

                GetTimeSyncService()->CompleteSimulationStep();
            });

    }

    void InitControllers() override
    {
    }


    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        std::stringstream ss;
        ss << std::left << std::setw(8) << "SimStep" << std::right << std::setw(10) << "T=" << now;
        GetLogger()->Info(ss.str());

        _now = now;

        DoWork();
    }

    void DoWorkAsync() override
    {
        DoWork();
        _now += GetArguments().duration;
    }

    void DoWork()
    {

    }

public:

};

int main(int argc, char** argv)
{
    std::string appDescription = "Counterpart of the CANoe time sync. test configuration for tests. Triggers CompleteSimStep via PubSub.";

    Arguments args;
    args.duration = 10ms;
    args.participantName = "P1";
    args.simStepHandlerAsync = true;
    args.asFastAsPossible = true;
    TestingSUT app{args};

    app.SetupCommandLineArgs(argc, argv, appDescription,
                             {ApplicationBase::DefaultArg::Async, ApplicationBase::DefaultArg::AsFastAsPossible});
    return app.Run();
}
