// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "PubSubDemoCommon.hpp"

class Publisher : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IDataPublisher* _gpsPublisher;
    IDataPublisher* _temperaturePublisher;

    void AddCommandLineArgs() override {}

    void EvaluateCommandLineArgs() override {}

    void CreateControllers() override
    {
        _gpsPublisher = GetParticipant()->CreateDataPublisher("GpsPublisher", PubSubDemoCommon::dataSpecGps, 0);
        _temperaturePublisher =
            GetParticipant()->CreateDataPublisher("TemperaturePublisher", PubSubDemoCommon::dataSpecTemperature, 0);
    }

    void InitControllers() override {}

    void PublishGPSData()
    {
        PubSubDemoCommon::GpsData gpsData;
        gpsData.latitude = 48.8235 + static_cast<double>((rand() % 150)) / 100000;
        gpsData.longitude = 9.0965 + static_cast<double>((rand() % 150)) / 100000;
        gpsData.signalQuality = "Strong";
        auto gpsSerialized = PubSubDemoCommon::SerializeGPSData(gpsData);

        std::stringstream ss;
        ss << "Publishing GPS data: lat=" << gpsData.latitude << ", lon=" << gpsData.longitude
           << ", signalQuality=" << gpsData.signalQuality;
        GetLogger()->Info(ss.str());

        _gpsPublisher->Publish(gpsSerialized);
    }

    void PublishTemperatureData()
    {
        double temperature = 25.0 + static_cast<double>(rand() % 10) / 10.0;
        auto temperatureSerialized = PubSubDemoCommon::SerializeTemperature(temperature);

        std::stringstream ss;
        ss << "Publishing temperature data: temperature=" << temperature;
        GetLogger()->Info(ss.str());

        _temperaturePublisher->Publish(temperatureSerialized);
    }

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override
    {
        PublishGPSData();
        PublishTemperatureData();
    }

    void DoWorkAsync() override
    {
        PublishGPSData();
        PublishTemperatureData();
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "Publisher";
    Publisher app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Publisher: Publish GPS and Temperature data");

    return app.Run();
}
