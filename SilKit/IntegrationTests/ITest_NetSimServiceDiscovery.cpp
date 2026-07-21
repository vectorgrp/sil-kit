// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Integration test: verify that the service-discovery C API correctly reports bus controllers as
// simulated (isSimulated=true) when a network simulator is present, without embedding this
// plumbing into the CAN/Ethernet/LIN/FlexRay routing tests.
//
// The test sets up the minimal scenario: one network simulator participant that takes over one CAN
// network ("CAN1"), one regular participant with a CAN controller on that network, and one
// autonomous service-discovery observer. After the simulation, the observer must have received at
// least one event for a CAN controller with isSimulated=true and the correct
// simulatingParticipantName.

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
    std::string primaryIdentifier; // networkName for bus controllers
    bool isSimulated{false};
    std::string simulatingParticipantName;
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
    ev.primaryIdentifier = d->primaryIdentifier ? d->primaryIdentifier : "";
    ev.isSimulated = d->isSimulated != SilKit_False;
    ev.simulatingParticipantName = d->simulatingParticipantName ? d->simulatingParticipantName : "";

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

// A network simulator takes over the "CAN1" network. A regular participant creates a CAN
// controller on that network. The service-discovery observer must see the CAN controller event
// with isSimulated=true and simulatingParticipantName equal to the network simulator's
// participant name — demonstrating that the new API can identify a network simulator without
// any dedicated "NetworkLink" kind or separate cross-referencing by the caller.
TEST_F(ITest_NetSimServiceDiscovery, can_controller_reported_as_simulated_when_netsim_present)
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

    // The isSimulated=true events for the CAN controller fire during the simulation (when the
    // Link service from the network simulator arrives). They are accumulated in ctx.events by
    // the observer's IO thread. Allow a brief settle window after Run() returns for any
    // in-flight IO delivery.
    {
        std::unique_lock<std::mutex> lk{ctx.mutex};
        const bool found = ctx.cv.wait_for(lk, 5s, [&] {
            return std::any_of(ctx.events.begin(), ctx.events.end(), [](const DiscoveryEvent& e) {
                return e.serviceKind == SilKit_Experimental_ServiceKind_CanController && e.isSimulated;
            });
        });
        ASSERT_TRUE(found) << "no CAN controller event with isSimulated=true was observed";

        const auto it = std::find_if(ctx.events.begin(), ctx.events.end(), [](const DiscoveryEvent& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_CanController && e.isSimulated;
        });
        EXPECT_EQ(it->simulatingParticipantName, netSimName)
            << "simulatingParticipantName must be the network simulator's participant name";
    }

    SilKit_Participant_Destroy(observer);
}

} // namespace
