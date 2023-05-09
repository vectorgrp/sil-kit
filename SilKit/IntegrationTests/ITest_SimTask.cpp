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
#include "silkit/util/PrintableHexString.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <vector>

namespace {

using namespace std::chrono_literals;

using SilKit::Services::Orchestration::OperationMode;

class Counter
{
    std::atomic<uint32_t> _major{0};
    std::atomic<uint32_t> _minor{0};

public:
    void IncrementMajor()
    {
        ++_major;
        _minor = 0;
    }

    void IncrementMinor() { ++_minor; }

    auto Value() const -> uint64_t { return (static_cast<uint64_t>(_major) << 32) | static_cast<uint64_t>(_minor); }

    auto Major() const -> uint32_t { return _major; }

    auto Minor() const -> uint32_t { return _minor; }

    void PublishValue(SilKit::Services::PubSub::IDataPublisher *publisher) const
    {
        const uint64_t value = Value();

        uint8_t bytes[8];
        for (unsigned i = 0; i < 8; ++i)
        {
            bytes[i] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
        }

        publisher->Publish(SilKit::Util::Span<const uint8_t>{bytes, sizeof(bytes)});
    }

    static auto ExtractValue(SilKit::Util::Span<const uint8_t> bytes) -> uint64_t
    {
        uint64_t value{0};

        for (unsigned i = 0; i < 8; ++i)
        {
            value |= (static_cast<uint64_t>(bytes[i]) << (i * 8));
        }

        return value;
    }

    static auto ExtractMajor(const uint64_t value) -> uint32_t { return static_cast<uint32_t>(value >> 32); }

    static auto ExtractMinor(const uint64_t value) -> uint32_t { return static_cast<uint32_t>(value & 0xFFFFFFFF); }
};

TEST(ITest_SimTask, blocking_during_simtask_does_not_affect_processing_order)
{
    // This test creates one publisher and one subscriber participant. Both use the "synchronous"
    // simulation step handler.
    //
    // The publisher sends two uint32_t values packaged as a single uint64_t value. This packaging is handled by the
    // Counter class above. The Major field counts the simulation step, the Minor field counts the individual message
    // sent in a single simulation step. Each time the Major value is incremented, the minor value resets.
    //
    // Neither the Major, nor the Minor are zero when the message is sent.
    //
    // When the publisher enters the simulation step handler with a particular now timestamp, it
    // - increments the Major value and resets the Minor value (1)
    // - increments the Minor value and publishes the resulting value
    // - blocks and signals a worker thread
    // - in the worker thread: increments the Minor value and publishes the resulting value (2)
    // - waits until the worker thread has completed
    // - increments the Minor value and publishes the resulting value (3)
    //
    // After the test completes (100ms virtual timestamp has been reached), the test code checks that the
    // messages were received in the correct order, i.e., the major and minor values (1, 2, 3) are
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

        Counter counter;

        using Mutex = std::mutex;
        using Lock = std::unique_lock<Mutex>;

        struct
        {
            Mutex mx;
            std::condition_variable cv;
            bool blocking{false};
            std::atomic<bool> running{true};
        } s;

        pTimeSyncService->SetSimulationStepHandler(
            [pLifecycleService, pPublisher, &counter, &s](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                if (!s.running)
                {
                    return;
                }

                if (now >= 100ms)
                {
                    pLifecycleService->Stop("test reached limit");
                    s.running = false;
                    s.cv.notify_all();
                    return;
                }
                
                counter.IncrementMajor();

                counter.IncrementMinor();
                counter.PublishValue(pPublisher);

                {
                    Lock lock{s.mx};
                    s.blocking = true;
                }
                s.cv.notify_all();

                // do nothing

                {
                    Lock lock{s.mx};
                    s.cv.wait(lock, [&s] {
                        return !s.blocking || !s.running;
                    });

                    if (!s.running)
                    {
                        return;
                    }
                }

                counter.IncrementMinor();
                counter.PublishValue(pPublisher);
            },
            1ms);

        auto completerDone = std::async(std::launch::async, [pPublisher, &counter, &s] {
            while (s.running)
            {
                Lock lock{s.mx};
                s.cv.wait(lock, [&s] {
                    return s.blocking || !s.running;
                });

                if (!s.running)
                {
                    break;
                }

                counter.IncrementMinor();
                counter.PublishValue(pPublisher);

                s.blocking = false;
                s.cv.notify_all();
            }
        });

        auto lifecycleDone = pLifecycleService->StartLifecycle();
        ASSERT_EQ(lifecycleDone.wait_for(5s), std::future_status::ready);
        ASSERT_EQ(completerDone.wait_for(5s), std::future_status::ready);
    });

    std::vector<uint64_t> received;

    auto subscriberDone = std::async(std::launch::async, [participantConfiguration, &registryUri, spec, &received] {
        const auto s = SilKit::CreateParticipant(participantConfiguration, "Sub", registryUri);
        const auto sLifecycleService = s->CreateLifecycleService({OperationMode::Coordinated});
        const auto sTimeSyncService = sLifecycleService->CreateTimeSyncService();

        s->CreateDataSubscriber("Sub", spec,
                                [&received](SilKit::Services::PubSub::IDataSubscriber *,
                                            const SilKit::Services::PubSub::DataMessageEvent &event) {
                                    ASSERT_EQ(event.data.size(), 8);
                                    const auto value = Counter::ExtractValue(event.data);
                                    ASSERT_GT(Counter::ExtractMajor(value), uint32_t{0})
                                        << SilKit::Util::AsHexString(event.data).WithSeparator(":");
                                    ASSERT_GT(Counter::ExtractMinor(value), uint32_t{0})
                                        << SilKit::Util::AsHexString(event.data).WithSeparator(":");
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

    ASSERT_EQ(controlDone.wait_for(5s), std::future_status::ready);
    controlDone.get();

    ASSERT_EQ(publisherDone.wait_for(5s), std::future_status::ready);
    publisherDone.get();

    ASSERT_EQ(subscriberDone.wait_for(5s), std::future_status::ready);
    subscriberDone.get();

    ASSERT_GE(received.size(), static_cast<size_t>(100 * 3));

    for (size_t major = 1; major <= received.size() / 3; ++major)
    {
        for (size_t minor = 1; minor <= 3; ++minor)
        {
            const auto receivedValue = received[(major - 1) * 3 + (minor - 1)];
            const auto receivedMajor = Counter::ExtractMajor(receivedValue);
            const auto receivedMinor = Counter::ExtractMinor(receivedValue);

            EXPECT_EQ(major, receivedMajor);
            EXPECT_EQ(minor, receivedMinor);
        }
    }
}

} // namespace
