// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/rpc/all.hpp"
#include "silkit/services/rpc/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace SilKit::Services::Rpc;

static std::ostream& operator<<(std::ostream& os, const SilKit::Util::Span<const uint8_t>& v)
{
    os << "[ ";
    for (auto i : v)
        os << static_cast<int>(i) << " ";
    os << "]";
    return os;
}

static std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& v)
{
    return os << SilKit::Util::ToSpan(v);
}

// This are the common data structures used in RpcServerDemo and RpcClientDemo
namespace RpcDemoCommon {

std::string mediaType{SilKit::Util::SerDes::MediaTypeRpc()};
RpcSpec rpcSpecSignalStrength{"GetSignalStrength", mediaType};
RpcSpec rpcSpecSort{"Sort", mediaType};

// ----------------------------------------------------------------
// Data structure, serialization and deserialization for Tuner Data
// ----------------------------------------------------------------

enum class TunerBand : uint8_t
{
    AM = 0,
    FM = 1
};

struct TunerData
{
    double frequency;
    TunerBand tunerBand;
};

std::vector<uint8_t> SerializeTunerData(const TunerData& tunerData)
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(tunerData.frequency);
    serializer.Serialize(static_cast<uint8_t>(tunerData.tunerBand), 8);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

TunerData DeserializeTunerData(const std::vector<uint8_t>& eventData)
{
    TunerData tunerData;

    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    deserializer.BeginStruct();
    tunerData.frequency = deserializer.Deserialize<double>();
    tunerData.tunerBand = static_cast<RpcDemoCommon::TunerBand>(deserializer.Deserialize<uint8_t>(8));
    deserializer.EndStruct();

    return tunerData;
}

// ----------------------------------------------------------------
// Serialization and deserialization for a double (signal strength)
// ----------------------------------------------------------------

std::vector<uint8_t> SerializeSignalStrength(double signalStrength)
{
    SilKit::Util::SerDes::Serializer signalStrengthSerializer;
    signalStrengthSerializer.Serialize(signalStrength);

    return signalStrengthSerializer.ReleaseBuffer();
}

double DeserializeSignalStrength(const std::vector<uint8_t>& eventData)
{
    double signalStrength;

    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    signalStrength = deserializer.Deserialize<double>();

    return signalStrength;
}

// ----------------------------------------------------------------
// Serialization and deserialization for a std::vector<uint8_t>
// ----------------------------------------------------------------

std::vector<uint8_t> SerializeSortData(const std::vector<uint8_t>& numberList)
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(numberList);

    return serializer.ReleaseBuffer();
}

std::vector<uint8_t> DeserializeSortData(const std::vector<uint8_t>& eventData)
{
    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    std::vector<uint8_t> numberList = deserializer.Deserialize<std::vector<uint8_t>>();

    return numberList;
}

bool EvaluateCallStatus(RpcCallResultEvent callResult, ILogger* logger)
{
    std::stringstream ss;
    bool isSuccessful = false;

    switch (callResult.callStatus)
    {
    case RpcCallStatus::Success:
    {
        isSuccessful = true;
        break;
    }
    case RpcCallStatus::ServerNotReachable:
    {
        ss << "Warning: Call " << callResult.userContext << " failed with RpcCallStatus::ServerNotReachable";
        break;
    }
    case RpcCallStatus::UndefinedError:
    {
        ss << "Warning: Call " << callResult.userContext << " failed with RpcCallStatus::UndefinedError";
        break;
    }
    case RpcCallStatus::InternalServerError:
    {
        ss << "Warning: Call " << callResult.userContext << " failed with RpcCallStatus::InternalServerError";
        break;
    }
    case RpcCallStatus::Timeout:
    {
        ss << "Warning: Call " << callResult.userContext << " failed with RpcCallStatus::Timeout";
        break;
    }
    default:
        break;
    }

    if (!isSuccessful)
    {
        logger->Info(ss.str());
    }

    return isSuccessful;
}

} // namespace RpcDemoCommon
