// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <numeric>

#include "CreateParticipant.hpp"
#include "VAsioRegistry.hpp"

#include "silkit/core/sync/all.hpp"
#include "silkit/services/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Config;
using namespace SilKit::Core;

TEST(SilKitRegistryReusageITest, DISABLED_SilKitRegistry_must_be_reusable_after_shutdown)
{
    //auto domainId = static_cast<uint32_t>(GetTestPid());

    //ConfigBuilder builder{"TestConfig"};
    //builder.SimulationSetup().AddParticipant("P1").AddParticipantController().WithSyncType(SyncType::DistributedTimeQuantum);
    //builder.SimulationSetup().AddParticipant("P2").AddParticipantController().WithSyncType(SyncType::DistributedTimeQuantum);
    //builder.SimulationSetup().ConfigureTimeSync().WithTickPeriod(1ms);
    //builder.WithActiveMiddleware(Middleware::VAsio);
    //auto config = builder.Build();

    //std::promise<void> allConnected;
    //std::promise<void> allDisconnected;
    //const auto numIterations = 5;

    //VAsioRegistry registry{config};
    //registry.SetAllConnectedHandler([&allConnected]() { std::cout << "connected\n";  allConnected.set_value(); });
    //registry.SetAllDisconnectedHandler([&allDisconnected]() { std::cout << "disconnected\n";  allDisconnected.set_value(); });
    //registry.ProvideDomain(domainId);

    //auto RunParticipant = [&config, domainId](auto name) {
    //    auto participant = CreateParticipantImpl(config, name);
    //    participant->JoinSilKitDomain(domainId);
    //    auto participantController = participant->GetParticipantController();
    //    participantController->SetSimulationTask([](auto /*now*/, auto /*duration*/) {});
    //    participantController->RunAsync();
    //    return participant;
    //};

    //for (auto i = 0; i < numIterations; i++)
    //{
    //    auto allConnectedFuture = allConnected.get_future();
    //    auto allDisconnectedFuture = allDisconnected.get_future();
    //    {
    //        auto p1 = RunParticipant("P1");
    //        auto p2 = RunParticipant("P2");
    //        auto status = allConnectedFuture.wait_for(60s);
    //        ASSERT_EQ(status, std::future_status::ready);
    //    }
    //    auto status = allDisconnectedFuture.wait_for(60s);
    //    ASSERT_EQ(status, std::future_status::ready);
    //    
    //    // reset promises
    //    allConnected = std::promise<void>{};
    //    allDisconnected = std::promise<void>{};
    //}
}

} // anonymous namespace
