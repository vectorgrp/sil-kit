/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SilKitEventQueue.hpp"

using namespace testing;

namespace SilKit {

namespace Dashboard {

class Test_DashboardSilKitEventQueue : public Test
{
public:
    std::shared_ptr<SilKitEventQueue> CreateService()
    {
        return std::make_shared<SilKitEventQueue>();
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
