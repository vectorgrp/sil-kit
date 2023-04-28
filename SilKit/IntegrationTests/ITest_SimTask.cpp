// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <vector>

namespace {

using namespace std::chrono_literals;

using SilKit::Services::Orchestration::OperationMode;

TEST(ITest_SimTask, blocking_during_simtask_does_not_affect_processing_order)
{
    // This test creates one publisher and one subscriber participant. Both use the "synchronous"
    // simulation step handler.
    //
    // When the publisher enters the simulation step handler with a particular now timestamp, it
    // - sends a message with contents: now, 1
    // - blocks and signals a worker thread
    // - in the worker thread: sends a message with contents: now, 2
    // - waits until the worker thread has completed
    // - sends a message with contents: now, 3
    //
    // After the test completes (100ms virtual timestamp has been reached), the test code checks that the
    // messages were received in the correct order, i.e., the timestamps and counter values (1, 2, 3) are
    // in the correct order.

    const auto registryParticipantConfiguration = SilKit::Config::ParticipantConfigurationFromString("");

    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryParticipantConfiguration);
    auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    const auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString("");

    auto controlDone = std::async(std::launch::async, [participantConfiguration, &registryUri] {
        const auto c = SilKit::CreateParticipant(participantConfiguration, "Control", registryUri);
        const auto cLifecycleService = c->CreateLifecycleService({OperationMode::Coordinated});
        const auto cTimeSyncService = cLifecycleService->CreateTimeSyncService();
        const auto cSystemController = SilKit::Experimental::Participant::CreateSystemController(c.get());

        SilKit::Services::Orchestration::WorkflowConfiguration workflowConfiguration{{"Pub", "Sub", "Control"}};
        cSystemController->SetWorkflowConfiguration(workflowConfiguration);

        cTimeSyncService->SetSimulationStepHandler(
            [](std::chrono::nanoseconds, std::chrono::nanoseconds) {
                // do nothing
            },
            1ms);

        auto lifecycleDone = cLifecycleService->StartLifecycle();
        ASSERT_EQ(lifecycleDone.wait_for(5s), std::future_status::ready);
    });

    SilKit::Services::PubSub::PubSubSpec spec{"Topic", "MediaType"};

    auto publisherDone = std::async(std::launch::async, [participantConfiguration, &registryUri, spec] {
        const auto p = SilKit::CreateParticipant(participantConfiguration, "Pub", registryUri);
        const auto pLifecycleService = p->CreateLifecycleService({OperationMode::Coordinated});
        const auto pTimeSyncService = pLifecycleService->CreateTimeSyncService();
        const auto pPublisher = p->CreateDataPublisher("Pub", spec);

        uint32_t counter = 0;

        using Mutex = std::mutex;
        using Lock = std::unique_lock<Mutex>;

        struct
        {
            Mutex mx;
            std::condition_variable cv;
            bool blocking;
            std::atomic<bool> completing{true};
        } s;

        pTimeSyncService->SetSimulationStepHandler(
            [pLifecycleService, pPublisher, &counter, &s](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                counter = (((counter >> (3 * 8)) & 0xFF) + 1) << (3 * 8);

                counter = (counter & 0xFF000000) | 1;
                pPublisher->Publish(SilKit::Util::Span<const uint8_t>{reinterpret_cast<const uint8_t *>(&counter), 4});

                if (now >= 100ms)
                {
                    pLifecycleService->Stop("test reached limit");
                    s.completing = false;
                }

                {
                    Lock lock{s.mx};
                    s.blocking = true;
                }
                s.cv.notify_one();

                // do nothing

                {
                    Lock lock{s.mx};
                    s.cv.wait(lock, [&s] {
                        return !s.blocking;
                    });
                }

                counter = (counter & 0xFF000000) | 3;
                pPublisher->Publish(SilKit::Util::Span<const uint8_t>{reinterpret_cast<const uint8_t *>(&counter), 4});
            },
            1ms);

        auto completerDone = std::async(std::launch::async, [pPublisher, &counter, &s] {
            while (s.completing)
            {
                Lock lock{s.mx};
                s.cv.wait(lock, [&s] {
                    return s.blocking;
                });

                counter = (counter & 0xFF000000) | 2;
                pPublisher->Publish(SilKit::Util::Span<const uint8_t>{reinterpret_cast<const uint8_t *>(&counter), 4});

                s.blocking = false;
                s.cv.notify_one();
            }
        });

        auto lifecycleDone = pLifecycleService->StartLifecycle();
        ASSERT_EQ(lifecycleDone.wait_for(5s), std::future_status::ready);
        ASSERT_EQ(completerDone.wait_for(5s), std::future_status::ready);
    });

    std::vector<uint32_t> received;

    auto subscriberDone = std::async(std::launch::async, [participantConfiguration, &registryUri, spec, &received] {
        const auto s = SilKit::CreateParticipant(participantConfiguration, "Sub", registryUri);
        const auto sLifecycleService = s->CreateLifecycleService({OperationMode::Coordinated});
        const auto sTimeSyncService = sLifecycleService->CreateTimeSyncService();

        s->CreateDataSubscriber("Sub", spec,
                                [&received](SilKit::Services::PubSub::IDataSubscriber *,
                                            const SilKit::Services::PubSub::DataMessageEvent &event) {
                                    const auto value = *reinterpret_cast<const uint32_t *>(event.data.data());
                                    received.emplace_back(value);
                                });

        sTimeSyncService->SetSimulationStepHandler(
            [](std::chrono::nanoseconds, std::chrono::nanoseconds) {
                // do nothing
            },
            1ms);

        auto lifecycleDone = sLifecycleService->StartLifecycle();

        ASSERT_EQ(lifecycleDone.wait_for(5s), std::future_status::ready);
    });

    controlDone.wait_for(5s);
    controlDone.get();

    publisherDone.wait_for(5s);
    publisherDone.get();

    subscriberDone.wait_for(5s);
    subscriberDone.get();

    ASSERT_GT(received.size(), static_cast<size_t>(100 * 3));

    for (size_t i = 0; i < received.size() / 3; ++i)
    {
        EXPECT_EQ((received[i * 3 + 0] >> (3 * 8)) & 0xFF, static_cast<uint32_t>(i) + 1);
        EXPECT_EQ((received[i * 3 + 1] >> (3 * 8)) & 0xFF, static_cast<uint32_t>(i) + 1);
        EXPECT_EQ((received[i * 3 + 2] >> (3 * 8)) & 0xFF, static_cast<uint32_t>(i) + 1);

        EXPECT_EQ(received[i * 3 + 0] & 0xFF, 1);
        EXPECT_EQ(received[i * 3 + 1] & 0xFF, 2);
        EXPECT_EQ(received[i * 3 + 2] & 0xFF, 3);
    }
}

} // namespace
