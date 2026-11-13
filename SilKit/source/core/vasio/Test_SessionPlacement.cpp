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

TEST_F(Test_SessionPlacement, participant_that_is_not_required_joins_a_generation_that_has_started)
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
