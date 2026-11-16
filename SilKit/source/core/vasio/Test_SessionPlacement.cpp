// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/SessionPlacement.hpp"
#include "core/vasio/VAsioReceiver.hpp"
#include "core/vasio/mock/FakeVAsioPeer.hpp"

#include <set>
#include <string>
#include <vector>

#include "services/logging/MockLogger.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace SilKit {
namespace Core {

struct SessionPlacementTestAccess
{
    static bool IsGenerationUsed(const SessionPlacement& placement, uint64_t generation)
    {
        return placement._generations.at(generation).used;
    }

    static bool HasGenerationState(const SessionPlacement& placement, uint64_t generation)
    {
        return placement._generations.count(generation) != 0;
    }
};

} // namespace Core
} // namespace SilKit

namespace {

using SilKit::Core::FakeVAsioPeer;
using SilKit::Core::SessionPlacement;
using SilKit::Core::SessionPlacementTestAccess;

class Test_SessionPlacement : public testing::Test
{
protected:
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;

    testing::NiceMock<SilKit::Services::Logging::MockLogger> _logger;
    SessionPlacement _placement{"Sim", &_logger};
    // the registered peers, which are owned by the connection in the registry
    SessionPlacement::Snapshot _registered;

    // The peer is registered after it was admitted, like the connection does it after the announcement.
    void Join(FakeVAsioPeer& peer)
    {
        _placement.Admit(&peer, _registered);
        _registered[*peer.GetServiceDescriptor().GetGeneration()].insert(peer.GetInfo().participantName);
    }

    void Leave(FakeVAsioPeer& peer)
    {
        const auto generation{*peer.GetServiceDescriptor().GetGeneration()};
        auto& names{_registered.at(generation)};
        _placement.OnPeerLeaving(&peer, names.size() == 1);
        names.erase(peer.GetInfo().participantName);
        if (names.empty())
        {
            _registered.erase(generation);
        }
    }

    void SendWorkflowConfiguration(FakeVAsioPeer& peer, std::vector<std::string> requiredParticipantNames)
    {
        SilKit::Services::Orchestration::WorkflowConfiguration configuration;
        configuration.requiredParticipantNames = std::move(requiredParticipantNames);
        _placement.OnWorkflow(&peer, configuration, _registered);
    }

    // Reports the participant states in the order of the lifecycle, up to and including the given state.
    void AdvanceTo(FakeVAsioPeer& peer, ParticipantState target)
    {
        for (const auto state :
             {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
              ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun, ParticipantState::Running})
        {
            SilKit::Services::Orchestration::ParticipantStatus status;
            status.participantName = peer.GetInfo().participantName;
            status.state = state;
            _placement.OnStatus(&peer, status, _registered);
            if (state == target)
            {
                return;
            }
        }
    }

    bool IsGenerationUsed(uint64_t generation) const
    {
        return SessionPlacementTestAccess::IsGenerationUsed(_placement, generation);
    }
};

TEST_F(Test_SessionPlacement, peers_with_the_same_name_are_placed_into_different_generations)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};
    FakeVAsioPeer b2{"B", "Sim"};

    Join(a1);
    Join(b1);
    Join(a2);
    Join(b2);

    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(b1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_EQ(b2.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, generation_gets_reset_when_none_exists)
{
    FakeVAsioPeer a{"A", "Sim"};

    Join(a);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);

    Leave(a);

    Join(a);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_SessionPlacement, rejoining_bumps_the_generation_once_the_simulation_has_started)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    Join(a);
    Join(b);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(b.GetServiceDescriptor().GetGeneration(), 1u);

    SendWorkflowConfiguration(a, {"A", "B"});
    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_FALSE(IsGenerationUsed(1));
    AdvanceTo(b, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));

    Leave(a);

    Join(a);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, rejoining_reuses_the_generation_before_the_simulation_has_started)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    Join(a);
    Join(b);

    SendWorkflowConfiguration(a, {"A", "B"});
    AdvanceTo(a, ParticipantState::ReadyToRun);
    AdvanceTo(b, ParticipantState::CommunicationInitialized);
    EXPECT_FALSE(IsGenerationUsed(1));

    Leave(a);
    Join(a);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_FALSE(IsGenerationUsed(1));

    // the status of A was dropped on shutdown, so B reaching ReadyToRun is not enough to start the simulation
    AdvanceTo(b, ParticipantState::ReadyToRun);
    EXPECT_FALSE(IsGenerationUsed(1));

    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
}

TEST_F(Test_SessionPlacement, coordinated_participant_does_not_join_a_generation_that_has_started)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    Join(a);
    SendWorkflowConfiguration(a, {"A", "B"});
    AdvanceTo(a, ParticipantState::Running);
    EXPECT_FALSE(IsGenerationUsed(1));

    // B is required, but has not joined yet: it can still join, the simulation did not start without it
    Join(b);
    EXPECT_EQ(b.GetServiceDescriptor().GetGeneration(), 1u);
    AdvanceTo(b, ParticipantState::Running);
    EXPECT_TRUE(IsGenerationUsed(1));

    Leave(b);
    Join(b);
    EXPECT_EQ(b.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, autonomous_participant_joins_running_generation)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer observer{"Observer", "Sim"};

    Join(a);
    SendWorkflowConfiguration(a, {"A"});
    AdvanceTo(a, ParticipantState::Running);
    EXPECT_TRUE(IsGenerationUsed(1));

    Join(observer);
    EXPECT_EQ(observer.GetServiceDescriptor().GetGeneration(), 1u);

    // hopping off and on again keeps it in the running simulation
    Leave(observer);
    Join(observer);
    EXPECT_EQ(observer.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_SessionPlacement, autonomous_participant_joins_most_recent_live_generation)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};
    FakeVAsioPeer b2{"B", "Sim"};
    FakeVAsioPeer observer{"C", "Sim"};

    Join(a1);
    Join(b1);
    SendWorkflowConfiguration(a1, {"A", "B"});
    AdvanceTo(a1, ParticipantState::Running);
    AdvanceTo(b1, ParticipantState::Running);
    ASSERT_TRUE(IsGenerationUsed(1));

    Join(a2);
    Join(b2);
    SendWorkflowConfiguration(a2, {"A", "B"});
    AdvanceTo(a2, ParticipantState::Running);
    AdvanceTo(b2, ParticipantState::Running);
    ASSERT_TRUE(IsGenerationUsed(2));

    Join(observer);
    EXPECT_EQ(observer.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, duplicate_autonomous_participant_spawns_new_generation)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};
    FakeVAsioPeer b2{"B", "Sim"};
    FakeVAsioPeer observer1{"C", "Sim"};
    FakeVAsioPeer observer2{"C", "Sim"};

    Join(a1);
    Join(b1);
    SendWorkflowConfiguration(a1, {"A", "B"});
    AdvanceTo(a1, ParticipantState::Running);
    AdvanceTo(b1, ParticipantState::Running);
    ASSERT_TRUE(IsGenerationUsed(1));

    Join(a2);
    Join(b2);
    SendWorkflowConfiguration(a2, {"A", "B"});
    AdvanceTo(a2, ParticipantState::Running);
    AdvanceTo(b2, ParticipantState::Running);
    ASSERT_TRUE(IsGenerationUsed(2));

    Join(observer1);
    ASSERT_EQ(observer1.GetServiceDescriptor().GetGeneration(), 2u);
    Join(observer2);
    EXPECT_EQ(observer2.GetServiceDescriptor().GetGeneration(), 3u);
    EXPECT_EQ(observer1.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, rejoining_autonomous_system_controller_spawns_new_generation)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};
    FakeVAsioPeer controller{"Controller", "Sim"};

    Join(a);
    Join(b);
    Join(controller);
    SendWorkflowConfiguration(controller, {"A", "B"});
    AdvanceTo(a, ParticipantState::Running);
    AdvanceTo(b, ParticipantState::Running);
    ASSERT_TRUE(IsGenerationUsed(1));

    Leave(controller);
    Join(controller);
    EXPECT_EQ(controller.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_SessionPlacement, changing_workflow_configuration_between_generations_is_rejected)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    for (const bool disconnectController : {false, true})
    {
        SCOPED_TRACE(disconnectController);
        FakeVAsioPeer a1{"A", "Sim"};
        FakeVAsioPeer b1{"B", "Sim"};
        FakeVAsioPeer controller1{"Controller", "Sim"};
        FakeVAsioPeer a2{"A", "Sim"};
        FakeVAsioPeer controller2{"Controller", "Sim"};

        Join(a1);
        Join(b1);
        Join(controller1);
        SendWorkflowConfiguration(controller1, {"A", "B"});
        AdvanceTo(a1, ParticipantState::Running);
        AdvanceTo(b1, ParticipantState::Running);
        ASSERT_TRUE(IsGenerationUsed(1));

        if (disconnectController)
        {
            Leave(controller1);
        }
        Join(a2);
        ASSERT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
        Join(controller2);

        EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(1);
        SendWorkflowConfiguration(controller2, {"A"});
        testing::Mock::VerifyAndClearExpectations(&_logger);
        EXPECT_FALSE(SessionPlacementTestAccess::HasGenerationState(_placement, 2));

        AdvanceTo(a2, ParticipantState::Running);
        EXPECT_FALSE(IsGenerationUsed(2));

        Leave(controller2);
        Leave(a2);
        if (!disconnectController)
        {
            Leave(controller1);
        }
        Leave(a1);
        Leave(b1);
    }
}

TEST_F(Test_SessionPlacement, changing_system_controller_name_between_generations_is_rejected)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    for (const bool disconnectController : {false, true})
    {
        SCOPED_TRACE(disconnectController);
        FakeVAsioPeer a1{"A", "Sim"};
        FakeVAsioPeer b1{"B", "Sim"};
        FakeVAsioPeer controller1{"Controller", "Sim"};
        FakeVAsioPeer a2{"A", "Sim"};
        FakeVAsioPeer controller2{"Controller2", "Sim"};

        Join(a1);
        Join(b1);
        Join(controller1);
        SendWorkflowConfiguration(controller1, {"A", "B"});
        AdvanceTo(a1, ParticipantState::Running);
        AdvanceTo(b1, ParticipantState::Running);
        ASSERT_TRUE(IsGenerationUsed(1));

        if (disconnectController)
        {
            Leave(controller1);
        }
        Join(a2);
        ASSERT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
        Join(controller2);

        EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(1);
        SendWorkflowConfiguration(controller2, {"A", "B"});
        testing::Mock::VerifyAndClearExpectations(&_logger);

        Leave(controller2);
        Leave(a2);
        if (!disconnectController)
        {
            Leave(controller1);
        }
        Leave(a1);
        Leave(b1);
    }
}

TEST_F(Test_SessionPlacement, equivalent_workflows_are_accepted_independently_in_each_generation)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};
    FakeVAsioPeer b2{"B", "Sim"};
    Join(a1);
    Join(b1);
    Join(a2);
    Join(b2);

    AdvanceTo(a2, ParticipantState::ReadyToRun);
    AdvanceTo(b2, ParticipantState::ReadyToRun);
    SendWorkflowConfiguration(a1, {"A", "B"});
    EXPECT_FALSE(IsGenerationUsed(1));
    EXPECT_FALSE(IsGenerationUsed(2));

    SendWorkflowConfiguration(a2, {"B", "A", "A"});
    EXPECT_FALSE(IsGenerationUsed(1));
    EXPECT_TRUE(IsGenerationUsed(2));
    SendWorkflowConfiguration(a2, {"A", "B"});

    AdvanceTo(a1, ParticipantState::ReadyToRun);
    AdvanceTo(b1, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
}

TEST_F(Test_SessionPlacement, rejected_workflows_preserve_the_contract_and_generation_state)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};
    FakeVAsioPeer controller{"Controller", "Sim"};
    FakeVAsioPeer otherController{"OtherController", "Sim"};
    Join(a);
    Join(b);
    Join(controller);
    Join(otherController);
    SendWorkflowConfiguration(controller, {"A", "B"});
    AdvanceTo(a, ParticipantState::ReadyToRun);

    EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(3);
    SendWorkflowConfiguration(controller, {"A"});
    SendWorkflowConfiguration(otherController, {"A", "B"});
    SendWorkflowConfiguration(otherController, {"A"});
    testing::Mock::VerifyAndClearExpectations(&_logger);
    EXPECT_FALSE(IsGenerationUsed(1));

    AdvanceTo(b, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    SendWorkflowConfiguration(controller, {"B", "A"});
}

TEST_F(Test_SessionPlacement, an_empty_workflow_still_establishes_a_contract)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    FakeVAsioPeer controller{"Controller", "Sim"};
    Join(controller);
    SendWorkflowConfiguration(controller, {});

    EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(1);
    SendWorkflowConfiguration(controller, {"A"});
    testing::Mock::VerifyAndClearExpectations(&_logger);

    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    SendWorkflowConfiguration(controller, {});
}

TEST_F(Test_SessionPlacement, invalid_workflow_senders_do_not_establish_a_contract)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    FakeVAsioPeer invalid{"Invalid", "Sim"};
    SilKit::Services::Orchestration::WorkflowConfiguration configuration;
    configuration.requiredParticipantNames = {"Invalid"};
    _placement.OnWorkflow(nullptr, configuration, _registered);
    _placement.OnWorkflow(&invalid, configuration, _registered);
    _placement.Admit(&invalid, _registered);
    _placement.OnWorkflow(&invalid, configuration, _registered);
    EXPECT_FALSE(SessionPlacementTestAccess::HasGenerationState(_placement, 1));

    FakeVAsioPeer a{"A", "Sim"};
    Join(a);
    EXPECT_CALL(_logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(_logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    SendWorkflowConfiguration(a, {"A"});
    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
}

TEST_F(Test_SessionPlacement, status_updates_before_the_workflow_configuration_are_considered)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    Join(a);
    Join(b);

    AdvanceTo(a, ParticipantState::ReadyToRun);
    AdvanceTo(b, ParticipantState::ReadyToRun);
    EXPECT_FALSE(IsGenerationUsed(1));

    SendWorkflowConfiguration(a, {"A", "B"});
    EXPECT_TRUE(IsGenerationUsed(1));
}

TEST_F(Test_SessionPlacement, participants_that_are_not_required_do_not_start_the_simulation)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer observer{"Observer", "Sim"};

    Join(a);
    Join(observer);

    SendWorkflowConfiguration(a, {"A"});
    AdvanceTo(observer, ParticipantState::Running);
    EXPECT_FALSE(IsGenerationUsed(1));

    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
}

TEST_F(Test_SessionPlacement, generations_start_independently)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};

    Join(a1);
    Join(a2);
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);

    SendWorkflowConfiguration(a1, {"A"});
    SendWorkflowConfiguration(a2, {"A"});
    AdvanceTo(a2, ParticipantState::Running);

    EXPECT_FALSE(IsGenerationUsed(1));
    EXPECT_TRUE(IsGenerationUsed(2));

    // the first generation is still open: a participant that is not yet in it is placed there
    FakeVAsioPeer b{"B", "Sim"};
    Join(b);
    EXPECT_EQ(b.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_SessionPlacement, rejoining_bumps_the_generation_while_participant_with_same_name_exists)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};

    Join(a1);
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);

    Join(a2);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);

    Leave(a1);

    Join(a1);
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 3u);
}

TEST_F(Test_SessionPlacement, admitting_a_peer_does_not_register_it)
{
    FakeVAsioPeer a{"A", "Sim"};

    _placement.Admit(&a, _registered);

    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_TRUE(_registered.empty());
    EXPECT_FALSE(SessionPlacementTestAccess::HasGenerationState(_placement, 1));
}

TEST_F(Test_SessionPlacement, state_of_a_generation_is_dropped_with_its_last_peer)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};

    Join(a1);
    Join(b1);
    Join(a2);
    SendWorkflowConfiguration(a1, {"A"});
    SendWorkflowConfiguration(a2, {"A"});
    ASSERT_TRUE(SessionPlacementTestAccess::HasGenerationState(_placement, 1));
    ASSERT_TRUE(SessionPlacementTestAccess::HasGenerationState(_placement, 2));

    Leave(a1);
    EXPECT_TRUE(SessionPlacementTestAccess::HasGenerationState(_placement, 1));

    Leave(b1);
    EXPECT_FALSE(SessionPlacementTestAccess::HasGenerationState(_placement, 1));
    EXPECT_TRUE(SessionPlacementTestAccess::HasGenerationState(_placement, 2));
}

TEST_F(Test_SessionPlacement, late_status_of_a_participant_that_left_is_ignored)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    Join(a);
    Join(b);
    SendWorkflowConfiguration(a, {"A", "B"});
    AdvanceTo(b, ParticipantState::ReadyToRun);

    Leave(a);

    // the status of the participant that left must not bring it back into the tracker
    AdvanceTo(b, ParticipantState::ReadyToRun);
    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_FALSE(IsGenerationUsed(1));

    Join(a);
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    AdvanceTo(a, ParticipantState::ReadyToRun);
    EXPECT_TRUE(IsGenerationUsed(1));
}

} // namespace
