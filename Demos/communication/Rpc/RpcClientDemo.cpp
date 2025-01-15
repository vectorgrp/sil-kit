// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "RpcDemoCommon.hpp"

class RpcClient : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IRpcClient* _rpcClientSignalStrength;
    IRpcClient* _rpcClientSort;
    uint16_t _callCounter = 0;

    void AddCommandLineArgs() override {}

    void EvaluateCommandLineArgs() override {}

    void CreateControllers() override
    {
        _rpcClientSignalStrength = GetParticipant()->CreateRpcClient(
            "ClientSignalStrength", RpcDemoCommon::rpcSpecSignalStrength,
            [this](IRpcClient* /*client*/, RpcCallResultEvent event) { CallReturnGetSignalStrength(event); });

        _rpcClientSort = GetParticipant()->CreateRpcClient(
            "ClientSort", RpcDemoCommon::rpcSpecSort,
            [this](IRpcClient* /*client*/, RpcCallResultEvent event) { CallReturnSort(event); });
    }

    void InitControllers() override {}

    void CallSignalStrength()
    {
        // Add an incrementing callCounter as userContext to identify the corresponding call on reception of a call result.
        const auto userContext = reinterpret_cast<void*>(uintptr_t(_callCounter++));

        RpcDemoCommon::TunerData randomTunerData;
        randomTunerData.tunerBand = RpcDemoCommon::TunerBand::FM;
        std::string band = "FM";
        randomTunerData.frequency = 87.0 + static_cast<double>(rand() % 100) / 100.0;
        auto tunerDataSerialized = RpcDemoCommon::SerializeTunerData(randomTunerData);

        std::stringstream ss;
        ss << "Calling 'SignalStrength' with arguments: band=" << band << ", frequency=" << randomTunerData.frequency
           << " (userContext=" << userContext << ")";
        GetLogger()->Info(ss.str());

        _rpcClientSignalStrength->Call(tunerDataSerialized, userContext);
    }

    void CallReturnGetSignalStrength(RpcCallResultEvent callResult)
    {
        if (RpcDemoCommon::EvaluateCallStatus(callResult, GetLogger()))
        {
            auto signalStrength =
                RpcDemoCommon::DeserializeSignalStrength(SilKit::Util::ToStdVector(callResult.resultData));

            std::stringstream ss;
            ss << "Call 'GetSignalStrength' returned: signalStrength=" << signalStrength
               << " (userContext=" << callResult.userContext << ")";
            GetLogger()->Info(ss.str());
        }
    }


    void CallSort()
    {
        // Add an incrementing callCounter as userContext to identify the corresponding call on reception of a call result.
        const auto userContext = reinterpret_cast<void*>(uintptr_t(_callCounter++));

        std::vector<uint8_t> randomNumberList{static_cast<uint8_t>(rand() % 10), static_cast<uint8_t>(rand() % 10),
                                              static_cast<uint8_t>(rand() % 10)};
        auto numberListSerialized = RpcDemoCommon::SerializeSortData(randomNumberList);

        std::stringstream ss;
        ss << "Calling 'Sort' with arguments: randomNumberList=" << randomNumberList << " (userContext=" << userContext
           << ")";
        GetLogger()->Info(ss.str());

        _rpcClientSort->Call(numberListSerialized, userContext);
    }

    void CallReturnSort(RpcCallResultEvent callResult)
    {
        if (RpcDemoCommon::EvaluateCallStatus(callResult, GetLogger()))
        {
            auto sortedNumbers = RpcDemoCommon::DeserializeSortData(SilKit::Util::ToStdVector(callResult.resultData));

            std::stringstream ss;
            ss << "Call 'Sort' returned: sortedNumbers=" << sortedNumbers << " (userContext=" << callResult.userContext
               << ")";
            GetLogger()->Info(ss.str());
        }
    }

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override
    {
        CallSignalStrength();
        CallSort();
    }

    void DoWorkAsync() override
    {
        CallSignalStrength();
        CallSort();
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "RpcClient";
    RpcClient app{args};
    app.SetupCommandLineArgs(
        argc, argv, "SIL Kit Demo - RPC Client: Call remote procedures with arguments and evaluate the return values");

    return app.Run();
}
