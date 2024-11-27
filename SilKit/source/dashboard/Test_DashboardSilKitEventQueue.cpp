// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SilKitEvent.hpp"
#include "LockedQueue.hpp"

using namespace testing;
using namespace VSilKit;

namespace SilKit {

namespace Dashboard {

class Test_DashboardSilKitEventQueue : public Test
{
public:
    std::shared_ptr<LockedQueue<SilKitEvent>> CreateService()
    {
        return std::make_shared<LockedQueue<SilKitEvent>>();
    }
};

TEST_F(Test_DashboardSilKitEventQueue, ManyProducersAndOneConsumer)
{
    // Arrange

    // Act
    size_t eventCount = 0;
    {
        const auto service = CreateService();

        // consumer
        auto consumer = std::thread([&]() {
            std::vector<SilKitEvent> events;
            while (service->DequeueAllInto(events))
            {
                eventCount += events.size();
                events.clear();
            }
        });

        // producers
        SimulationStart simulationStart{"silkit://localhost:8500", 123456};
        service->Enqueue(SilKitEvent({}, simulationStart));
        std::vector<std::thread> producers;
        for (int participantIndex = 0; participantIndex < 50; ++participantIndex)
        {
            producers.push_back(std::thread([&service]() {
                Services::Orchestration::ParticipantConnectionInformation participantConnectionInformation;
                service->Enqueue(SilKitEvent({}, participantConnectionInformation));
                Services::Orchestration::ParticipantStatus participantStatus;
                service->Enqueue(SilKitEvent({}, participantStatus));
            }));
        }
        for (auto& producer : producers)
        {
            producer.join();
        }
        Services::Orchestration::SystemState systemState{};
        service->Enqueue(SilKitEvent({}, systemState));
        SimulationEnd simulatioEnd{456789};
        service->Enqueue(SilKitEvent({}, simulatioEnd));

        service->Stop();

        consumer.join();
    }

    // Assert
    ASSERT_EQ(eventCount, 103u) << "Wrong event count!";
}

} // namespace Dashboard
} // namespace SilKit
