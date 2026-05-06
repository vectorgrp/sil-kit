// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/pubsub/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace SilKit::Services::PubSub;

// This are the common data structures used in PublisherDemo and SubscriberDemo
namespace PubSubDemoCommon {

inline PubSubSpec MakeGpsSpec()
{
    return {"Gps", SilKit::Util::SerDes::MediaTypeData()};
}

inline PubSubSpec MakeTemperatureSpec()
{
    return {"Temperature", SilKit::Util::SerDes::MediaTypeData()};
}

// ----------------------------------------------------------------
// Data structure, serialization and deserialization for GPS Data
// ----------------------------------------------------------------

struct GpsData
{
    double latitude;
    double longitude;
    std::string signalQuality;
};

inline std::vector<uint8_t> SerializeGPSData(const PubSubDemoCommon::GpsData& gpsData)
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(gpsData.latitude);
    serializer.Serialize(gpsData.longitude);
    serializer.Serialize(gpsData.signalQuality);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

inline PubSubDemoCommon::GpsData DeserializeGPSData(const std::vector<uint8_t>& eventData)
{
    PubSubDemoCommon::GpsData gpsData;

    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    deserializer.BeginStruct();
    gpsData.latitude = deserializer.Deserialize<double>();
    gpsData.longitude = deserializer.Deserialize<double>();
    gpsData.signalQuality = deserializer.Deserialize<std::string>();
    deserializer.EndStruct();

    return gpsData;
}

// -----------------------------------------------------------------
// Serialization and deserialization for a double (temperature)
// -----------------------------------------------------------------

inline std::vector<uint8_t> SerializeTemperature(double temperature)
{
    SilKit::Util::SerDes::Serializer temperatureSerializer;
    temperatureSerializer.Serialize(temperature);

    return temperatureSerializer.ReleaseBuffer();
}

inline double DeserializeTemperature(const std::vector<uint8_t>& eventData)
{
    double temperature;

    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    temperature = deserializer.Deserialize<double>();

    return temperature;
}

} // namespace PubSubDemoCommon
