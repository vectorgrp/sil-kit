// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "PubSubDemoCommon.hpp"

class Subscriber : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    IDataSubscriber* _gpsSubscriber;
    IDataSubscriber* _temperatureSubscriber;

    void AddCommandLineArgs() override {}

    void EvaluateCommandLineArgs() override {}

    void CreateControllers() override
    {
        _gpsSubscriber = GetParticipant()->CreateDataSubscriber(
            "GpsSubscriber", PubSubDemoCommon::dataSpecGps,
            [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            auto gpsData = PubSubDemoCommon::DeserializeGPSData(SilKit::Util::ToStdVector(dataMessageEvent.data));

            std::stringstream ss;
            ss << "Received GPS data: lat=" << gpsData.latitude << ", lon=" << gpsData.longitude
               << ", signalQuality=" << gpsData.signalQuality;
            GetLogger()->Info(ss.str());
        });

        _temperatureSubscriber = GetParticipant()->CreateDataSubscriber(
            "TemperatureSubscriber", PubSubDemoCommon::dataSpecTemperature,
            [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            double temperature =
                PubSubDemoCommon::DeserializeTemperature(SilKit::Util::ToStdVector(dataMessageEvent.data));

            std::stringstream ss;
            ss << "Received temperature data: temperature=" << temperature;
            GetLogger()->Info(ss.str());
        });
    }

    void InitControllers() override {}

    void DoWorkSync(std::chrono::nanoseconds /*now*/) override {}

    void DoWorkAsync() override {}
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "Subscriber";
    Subscriber app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Subscriber: Receive GPS and Temperature data");

    return app.Run();
}
