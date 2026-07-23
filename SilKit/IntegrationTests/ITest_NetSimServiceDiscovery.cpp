// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Integration test: verify that the service-discovery C API reports a network simulator as a Link
// service (SilKit_Experimental_ServiceKind_Link) whose primaryIdentifier is the simulated network
// name and whose participantName is the simulating participant. A consumer joins that network name
// against a bus controller's primaryIdentifier to learn the controller is simulated -- without any
// dedicated cross-referencing being done inside the API.
//
// The test sets up the minimal scenario: one network simulator participant that takes over one CAN
// network ("CAN1"), one regular participant with a CAN controller on that network, and one
// autonomous service-discovery observer.

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "ITestFixture.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/capi/SilKit.h"
#include "silkit/experimental/netsim/all.hpp"
#include "silkit/services/can/all.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Tests;
using namespace SilKit::Experimental::NetworkSimulation;
using namespace SilKit::Experimental::NetworkSimulation::Can;

// ---------------------------------------------------------------------------
// Minimal simulated CAN network — no message routing needed for this test.
// ---------------------------------------------------------------------------

class MinimalSimulatedCanController : public ISimulatedCanController
{
public:
    void OnSetControllerMode(const CanControllerMode&) override {}
    void OnSetBaudrate(const CanConfigureBaudrate&) override {}
    void OnFrameRequest(const CanFrameRequest&) override {}
};

class MinimalSimulatedCanNetwork : public ISimulatedNetwork
{
    std::unique_ptr<IEventProducer> _producer;
    std::vector<std::unique_ptr<MinimalSimulatedCanController>> _controllers;

public:
    void SetEventProducer(std::unique_ptr<IEventProducer> producer) override
    {
        _producer = std::move(producer);
    }

    auto ProvideSimulatedController(ControllerDescriptor) -> ISimulatedController* override
    {
        _controllers.push_back(std::make_unique<MinimalSimulatedCanController>());
        return _controllers.back().get();
    }

    void SimulatedControllerRemoved(ControllerDescriptor) override {}
};

// ---------------------------------------------------------------------------
// Observer state — accumulated service-discovery events from the C API.
// ---------------------------------------------------------------------------

struct DiscoveryEvent
{
    SilKit_Experimental_ServiceKind serviceKind{};
    SilKit_Experimental_ServiceDiscoveryEvent_Type eventType{};
    std::string participantName;
    std::string primaryIdentifier; // networkName for bus controllers and network-simulator links
};

struct DiscoveryCtx
{
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<DiscoveryEvent> events;
};

void SilKitCALL OnNetSimDiscovery(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
                                  const SilKit_Experimental_ServiceDescriptor* d)
{
    auto* ctx = static_cast<DiscoveryCtx*>(context);

    DiscoveryEvent ev{};
    ev.serviceKind = d->serviceKind;
    ev.eventType = eventType;
    ev.participantName = d->participantName ? d->participantName : "";
    ev.primaryIdentifier = d->primaryIdentifier ? d->primaryIdentifier : "";

    {
        std::lock_guard<std::mutex> lk{ctx->mutex};
        ctx->events.push_back(std::move(ev));
    }
    ctx->cv.notify_all();
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class ITest_NetSimServiceDiscovery : public ITest_SimTestHarness
{
};

// A network simulator takes over the "CAN1" network. A regular participant creates a CAN controller
// on that network. The service-discovery observer must see a Link service whose primaryIdentifier is
// "CAN1" and whose participantName is the network simulator's participant name — demonstrating that
// the API surfaces a network simulator as a Link, joinable to bus controllers by network name.
TEST_F(ITest_NetSimServiceDiscovery, netsim_reported_as_link_service)
{
    const std::string netSimName = "NetworkSimulator";
    const std::string canPartName = "CanParticipant";
    const std::string networkName = "CAN1";

    SetupFromParticipantList({netSimName, canPartName});

    // Service-discovery observer: autonomous participant, NOT part of the sync group.
    // Creates a service discovery handle and registers a handler before the simulation starts,
    // so it sees all service announcements as participants join.
    DiscoveryCtx ctx;
    SilKit_ParticipantConfiguration* cfg{nullptr};
    ASSERT_EQ(SilKit_ParticipantConfiguration_FromString(&cfg, ""), SilKit_ReturnCode_SUCCESS);
    SilKit_Participant* observer{nullptr};
    ASSERT_EQ(SilKit_Participant_Create(&observer, cfg, "Observer", _registryUri.c_str()),
              SilKit_ReturnCode_SUCCESS);
    SilKit_ParticipantConfiguration_Destroy(cfg);

    SilKit_Experimental_ServiceDiscovery* sd{nullptr};
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&sd, observer), SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(sd, &ctx, &OnNetSimDiscovery),
              SilKit_ReturnCode_SUCCESS);

    // Network simulator participant: claims the CAN network and stops the simulation after 5 ms.
    {
        auto* p = _simTestHarness->GetParticipant(netSimName);
        auto* lc = p->GetOrCreateLifecycleService();
        auto* ts = p->GetOrCreateTimeSyncService();
        auto* netsim = p->GetOrCreateNetworkSimulator();

        netsim->SimulateNetwork(networkName, SimulatedNetworkType::CAN,
                                std::make_unique<MinimalSimulatedCanNetwork>());
        netsim->Start();

        ts->SetSimulationStepHandler(
            [lc](auto now, auto) {
            if (now >= 5ms)
                lc->Stop("done");
        }, 1ms);
    }

    // Regular participant: creates one CAN controller on the simulated network.
    {
        auto* p = _simTestHarness->GetParticipant(canPartName);
        auto* ts = p->GetOrCreateTimeSyncService();
        p->Participant()->CreateCanController("CanCtrl", networkName);
        ts->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    }

    auto ok = _simTestHarness->Run(10s);
    ASSERT_TRUE(ok) << "simulation should complete without timeout";

    // The Link service for the network simulator fires during the simulation. It is accumulated in
    // ctx.events by the observer's IO thread. Allow a brief settle window after Run() returns for
    // any in-flight IO delivery.
    {
        std::unique_lock<std::mutex> lk{ctx.mutex};
        const bool found = ctx.cv.wait_for(lk, 5s, [&] {
            return std::any_of(ctx.events.begin(), ctx.events.end(), [&](const DiscoveryEvent& e) {
                return e.serviceKind == SilKit_Experimental_ServiceKind_Link
                       && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated
                       && e.primaryIdentifier == networkName && e.participantName == netSimName;
            });
        });
        ASSERT_TRUE(found)
            << "no Link service with the network name and the network simulator participant was observed";

        // The CAN controller is reported as its own kind on the same network; joining on
        // primaryIdentifier lets a consumer conclude the controller is simulated.
        const bool sawController = std::any_of(ctx.events.begin(), ctx.events.end(), [&](const DiscoveryEvent& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_CanController
                   && e.primaryIdentifier == networkName;
        });
        EXPECT_TRUE(sawController) << "the CAN controller on the simulated network was not observed";
    }

    SilKit_Participant_Destroy(observer);
}

} // namespace
