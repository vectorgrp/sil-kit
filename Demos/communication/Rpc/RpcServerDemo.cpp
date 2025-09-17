// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "RpcDemoCommon.hpp"
#include <cmath>

class RpcServer : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IRpcServer* _rpcServerSignalStrength;
    IRpcServer* _rpcServerSort;


    void AddCommandLineArgs() override {}

    void EvaluateCommandLineArgs() override {}

    void CreateControllers() override
    {
        _rpcServerSignalStrength =
            GetParticipant()->CreateRpcServer("ServerSignalStrength", RpcDemoCommon::rpcSpecSignalStrength,
                                              [this](IRpcServer* server, RpcCallEvent event) {
            auto tunerData = RpcDemoCommon::DeserializeTunerData(SilKit::Util::ToStdVector(event.argumentData));

            // Calculation
            auto calcDummySignalStrength = [](double frequency, double peakFreq) {
                return exp(-(50.0 + static_cast<double>(rand() % 10)) * pow(frequency - peakFreq, 2));
            };

            double signalStrength = 0;
            std::string band = "";

            switch (tunerData.tunerBand)
            {
            case RpcDemoCommon::TunerBand::AM:
                band = "AM";
                signalStrength = calcDummySignalStrength(tunerData.frequency, 0.558);
                break;
            case RpcDemoCommon::TunerBand::FM:
                band = "FM";
                signalStrength = calcDummySignalStrength(tunerData.frequency, 87.6);
                break;
            default:
                break;
            }

            std::stringstream ss;
            ss << "Receive call to 'GetSignalStrength' with arguments: band=" << band
               << ", frequency=" << tunerData.frequency << "; returning signalStrength=" << signalStrength;
            GetLogger()->Info(ss.str());

            // Serialize result data
            SilKit::Util::SerDes::Serializer serializer;
            serializer.Serialize(signalStrength);

            // Submit call result to client
            server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
        });

        _rpcServerSort = GetParticipant()->CreateRpcServer("ServerSort", RpcDemoCommon::rpcSpecSort,
                                                           [this](IRpcServer* server, RpcCallEvent event) {
            // Deserialize incoming data
            auto argumentData = RpcDemoCommon::DeserializeSortData(SilKit::Util::ToStdVector(event.argumentData));

            // Calculation
            std::vector<uint8_t> resultData(argumentData);
            std::sort(resultData.begin(), resultData.end());

            std::stringstream ss;
            ss << "Receive call to 'Sort' with argumentData=" << argumentData
               << "; returning resultData=" << resultData;
            GetLogger()->Info(ss.str());

            // Serialize result data
            SilKit::Util::SerDes::Serializer serializer;
            serializer.Serialize(resultData);

            // Submit call result to client
            server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
        });
    }

    void InitControllers() override {}

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override {}

    void DoWorkAsync() override {}
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "RpcServer";
    RpcServer app{args};
    app.SetupCommandLineArgs(
        argc, argv,
        "SIL Kit Demo - RPC Server: Provide remotely accessible procedures with arguments and return values");

    return app.Run();
}
