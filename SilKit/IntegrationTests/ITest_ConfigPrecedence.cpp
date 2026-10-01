// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Values in the participant configuration take precedence over the values passed to the API.

#include <chrono>
#include <future>
#include <string>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Can;
using namespace SilKit::Services::Orchestration;

class ITest_ConfigPrecedence : public testing::Test
{
protected:
    void SetUp() override
    {
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
        _registryUri = _registry->StartListening("silkit://127.0.0.1:0");
    }

    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::string _registryUri;
};

TEST_F(ITest_ConfigPrecedence, participant_name_from_configuration_overrides_api_argument)
{
    auto observer = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Observer",
                                              _registryUri);
    auto* systemMonitor = observer->CreateSystemMonitor();

    std::promise<std::string> connectedPromise;
    bool connectedSeen = false;
    systemMonitor->SetParticipantConnectedHandler([&](const ParticipantConnectionInformation& info) {
        if (info.participantName != "Observer" && !connectedSeen)
        {
            connectedSeen = true;
            connectedPromise.set_value(info.participantName);
        }
    });

    auto participant = SilKit::CreateParticipant(
        SilKit::Config::ParticipantConfigurationFromString("ParticipantName: NameFromConfiguration"), "NameFromApi",
        _registryUri);

    auto connected = connectedPromise.get_future();
    ASSERT_EQ(connected.wait_for(10s), std::future_status::ready);
    EXPECT_EQ(connected.get(), "NameFromConfiguration");
    EXPECT_FALSE(systemMonitor->IsParticipantConnected("NameFromApi"));
}

TEST_F(ITest_ConfigPrecedence, registry_uri_from_configuration_overrides_api_argument)
{
    // Nothing listens on port 1. If the API argument were used, creating the participant would throw.
    const auto unreachableRegistryUri = "silkit://127.0.0.1:1";
    const auto config = R"(
Middleware:
  RegistryUri: )" + _registryUri + "\n";

    std::unique_ptr<SilKit::IParticipant> participant;
    EXPECT_NO_THROW(participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(config),
                                                            "Participant", unreachableRegistryUri));
    EXPECT_NE(participant, nullptr);
}

TEST_F(ITest_ConfigPrecedence, api_registry_uri_is_used_if_configuration_has_none)
{
    std::unique_ptr<SilKit::IParticipant> participant;
    EXPECT_NO_THROW(participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                            "Participant", _registryUri));
}

// The CAN controller of the writer is created on network "NetworkFromApi", but its configuration moves it to
// "SharedNetwork", where the reader listens. The frame only arrives if the configured network is used.
TEST(ITest_ConfigPrecedenceNetwork, controller_network_from_configuration_overrides_api_argument)
{
    const auto writerConfig = R"(
CanControllers:
- Name: CAN1
  Network: SharedNetwork
)";

    SilKit::Tests::SimTestHarnessArgs args;
    args.syncParticipantNames = {"CanWriter", "CanReader"};
    args.deferParticipantCreation = true;
    SilKit::Tests::SimTestHarness testHarness{args};

    auto* writer = testHarness.GetParticipant("CanWriter", writerConfig);
    auto* writerController = writer->Participant()->CreateCanController("CAN1", "NetworkFromApi");
    writer->GetOrCreateLifecycleService()->SetCommunicationReadyHandler([writerController] {
        writerController->SetBaudRate(500'000, 0, 0);
        writerController->Start();
    });
    writer->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [writerController](std::chrono::nanoseconds, std::chrono::nanoseconds) {
        CanFrame frame{};
        frame.canId = 42;
        frame.dlc = 1;
        std::array<uint8_t, 1> payload{0x2a};
        frame.dataField = SilKit::Util::MakeSpan(payload);
        writerController->SendFrame(frame);
    }, 1ms);

    auto* reader = testHarness.GetParticipant("CanReader", "");
    auto* readerController = reader->Participant()->CreateCanController("CAN1", "SharedNetwork");
    reader->GetOrCreateLifecycleService()->SetCommunicationReadyHandler([readerController] {
        readerController->SetBaudRate(500'000, 0, 0);
        readerController->Start();
    });
    reader->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [reader](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        // Stop in any case, so a wrong network fails the expectation below instead of timing out.
        if (now >= 50ms)
        {
            reader->Stop();
        }
    }, 1ms);

    bool received = false;
    readerController->AddFrameHandler([reader, &received](ICanController*, const CanFrameEvent& event) {
        if (event.frame.canId == 42 && !received)
        {
            received = true;
            reader->Stop();
        }
    });

    ASSERT_TRUE(testHarness.Run(10s));
    EXPECT_TRUE(received) << "The writer must send on the network from its configuration";
}

} // namespace
