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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "MockCapiTest.hpp"

#include <algorithm>

namespace {

using testing::DoAll;
using testing::Return;
using testing::SetArgPointee;
using testing::StrEq;

using namespace SilKit::Services::Orchestration;

MATCHER_P(LifecycleConfigurationMatcher, lifecycleConfigurationParam, "")
{
    const LifecycleConfiguration& cppLifecycleConfiguration = lifecycleConfigurationParam;
    const SilKit_LifecycleConfiguration* cLifecycleConfiguration = arg;

    if (cLifecycleConfiguration == nullptr)
    {
        *result_listener << "cLifecycleConfiguration must not be nullptr";
        return false;
    }

    if (cLifecycleConfiguration->operationMode
        != static_cast<SilKit_OperationMode>(cppLifecycleConfiguration.operationMode))
    {
        *result_listener << "operationMode does not match";
        return false;
    }

    return true;
}

MATCHER_P(WorkflowConfigurationMatcher, workflowConfigurationParam, "")
{
    const WorkflowConfiguration& cppWorkflowConfiguration = workflowConfigurationParam;
    const SilKit_WorkflowConfiguration* cWorkflowConfiguration = arg;

    if (cWorkflowConfiguration == nullptr)
    {
        *result_listener << "cWorkflowConfiguration must not be nullptr";
        return false;
    }

    if (cWorkflowConfiguration->requiredParticipantNames == nullptr)
    {
        *result_listener << "cWorkflowConfiguration->requiredParticipantNames must not be nullptr";
        return false;
    }

    if (cWorkflowConfiguration->requiredParticipantNames->numStrings
        != cppWorkflowConfiguration.requiredParticipantNames.size())
    {
        *result_listener << "number-of-required-participants does not match";
        return false;
    }

    std::set<std::string> cppRequiredParticipantNames{cppWorkflowConfiguration.requiredParticipantNames.begin(),
                                                      cppWorkflowConfiguration.requiredParticipantNames.end()};

    std::set<std::string> cRequiredParticipantNames{};
    std::transform(cWorkflowConfiguration->requiredParticipantNames->strings,
                   cWorkflowConfiguration->requiredParticipantNames->strings
                       + cWorkflowConfiguration->requiredParticipantNames->numStrings,
                   std::inserter(cRequiredParticipantNames, cRequiredParticipantNames.begin()), [](char* name) {
                       return std::string{name};
                   });

    if (cRequiredParticipantNames != cppRequiredParticipantNames)
    {
        *result_listener << "required-participant-names do not match";
        return false;
    }

    return true;
}

MATCHER_P(CppParticipantStatusMatcher, cParticipantStatusParam, "")
{
    const SilKit_ParticipantStatus* cParticipantStatus = cParticipantStatusParam;
    const ParticipantStatus& cppParticipantStatus = arg;

    if (cppParticipantStatus.participantName != cParticipantStatus->participantName)
    {
        *result_listener << "participantName does not match";
        return false;
    }

    if (cppParticipantStatus.enterReason != cParticipantStatus->enterReason)
    {
        *result_listener << "enterReason does not match";
        return false;
    }

    const auto cppEnterTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(cppParticipantStatus.enterTime.time_since_epoch());
    if (static_cast<SilKit_NanosecondsWallclockTime>(cppEnterTime.count()) != cParticipantStatus->enterTime)
    {
        *result_listener << "enterTime does not match";
        return false;
    }

    const auto cppRefreshTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(cppParticipantStatus.refreshTime.time_since_epoch());
    if (static_cast<SilKit_NanosecondsWallclockTime>(cppRefreshTime.count()) != cParticipantStatus->refreshTime)
    {
        *result_listener << "refreshTime does not match";
        return false;
    }

    if (cppParticipantStatus.state != static_cast<ParticipantState>(cParticipantStatus->participantState))
    {
        *result_listener << "participantState does not match";
        return false;
    }

    return true;
}

class HourglassOrchestrationTest : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_Participant* mockParticipant{reinterpret_cast<SilKit_Participant*>(uintptr_t(0x12345678))};
    SilKit_LifecycleService* mockLifecycleService{reinterpret_cast<SilKit_LifecycleService*>(uintptr_t(0x78563412))};
    SilKit_TimeSyncService* mockTimeSyncService{reinterpret_cast<SilKit_TimeSyncService*>(uintptr_t(0x87654321))};
    SilKit_SystemMonitor* mockSystemMonitor{reinterpret_cast<SilKit_SystemMonitor*>(uintptr_t(0x18273645))};
    SilKit_Experimental_SystemController* mockSystemController{
        reinterpret_cast<SilKit_Experimental_SystemController*>(uintptr_t(0x45362718))};

    HourglassOrchestrationTest()
    {
        using testing::_;
        ON_CALL(capi, SilKit_LifecycleService_Create(_, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockLifecycleService), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_TimeSyncService_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockTimeSyncService), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_SystemMonitor_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockSystemMonitor), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Experimental_SystemController_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockSystemController), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

// LifecycleService

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_Create)
{
    LifecycleConfiguration lifecycleConfiguration{};
    lifecycleConfiguration.operationMode = OperationMode::Coordinated;

    EXPECT_CALL(capi, SilKit_LifecycleService_Create(testing::_, mockParticipant,
                                                     LifecycleConfigurationMatcher(lifecycleConfiguration)));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, lifecycleConfiguration};
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetCommunicationReadyHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi,
                SilKit_LifecycleService_SetCommunicationReadyHandler(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetCommunicationReadyHandler([] {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetCommunicationReadyHandlerAsync)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(
        capi, SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetCommunicationReadyHandlerAsync([] {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(mockLifecycleService));

    lifecycleService.CompleteCommunicationReadyHandlerAsync();
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetStartingHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_SetStartingHandler(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetStartingHandler([] {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetStopHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_SetStopHandler(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetStopHandler([] {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetShutdownHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_SetShutdownHandler(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetShutdownHandler([] {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_SetAbortHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_SetAbortHandler(mockLifecycleService, testing::_, testing::_));

    lifecycleService.SetAbortHandler([](SilKit::Services::Orchestration::ParticipantState) {
        // do nothing
    });
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_StartLifecycle_WaitForLifecycleToComplete)
{
    const auto participantState{ParticipantState::Running};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_StartLifecycle(mockLifecycleService));
    EXPECT_CALL(capi, SilKit_LifecycleService_WaitForLifecycleToComplete(mockLifecycleService, testing::_))
        .WillOnce(DoAll(SetArgPointee<1>(static_cast<SilKit_ParticipantState>(participantState)),
                        Return(SilKit_ReturnCode_SUCCESS)));

    auto future = lifecycleService.StartLifecycle();
    EXPECT_EQ(future.get(), participantState);
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_ReportError)
{
    const std::string reason{"Reason"};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_ReportError(mockLifecycleService, StrEq(reason)));

    lifecycleService.ReportError(reason);
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_Pause)
{
    const std::string reason{"Reason"};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_Pause(mockLifecycleService, StrEq(reason)));

    lifecycleService.Pause(reason);
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_Continue)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_Continue(mockLifecycleService));

    lifecycleService.Continue();
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_Stop)
{
    const std::string reason{"Reason"};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_Stop(mockLifecycleService, StrEq(reason)));

    lifecycleService.Stop(reason);
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_State)
{
    const auto participantState{ParticipantState::Running};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_State(testing::_, mockLifecycleService))
        .WillOnce(DoAll(SetArgPointee<0>(static_cast<SilKit_ParticipantState>(participantState)),
                        Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_EQ(lifecycleService.State(), participantState);
}

TEST_F(HourglassOrchestrationTest, SilKit_LifecycleService_Status)
{
    const std::string participantName{"Participant1"};
    const std::string enterReason{"EnterReason"};
    const auto participantState{ParticipantState::Running};

    SilKit_ParticipantStatus cParticipantStatus{};
    SilKit_Struct_Init(SilKit_ParticipantStatus, cParticipantStatus);
    cParticipantStatus.participantName = participantName.c_str();
    cParticipantStatus.enterReason = enterReason.c_str();
    cParticipantStatus.participantState = static_cast<SilKit_ParticipantState>(participantState);
    cParticipantStatus.enterTime = UINT64_C(1000000000);
    cParticipantStatus.refreshTime = UINT64_C(2000000000);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::LifecycleService lifecycleService{
        mockParticipant, LifecycleConfiguration{OperationMode::Coordinated}};

    EXPECT_CALL(capi, SilKit_LifecycleService_Status(testing::_, mockLifecycleService))
        .WillOnce(DoAll(SetArgPointee<0>(cParticipantStatus), Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_THAT(lifecycleService.Status(), CppParticipantStatusMatcher(&cParticipantStatus));
}

// TimeSyncService

TEST_F(HourglassOrchestrationTest, SilKit_TimeSyncService_Create)
{
    EXPECT_CALL(capi, SilKit_TimeSyncService_Create(testing::_, mockLifecycleService));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::TimeSyncService timeSyncService{
        mockLifecycleService};
}

TEST_F(HourglassOrchestrationTest, SilKit_TimeSyncService_SetSimulationStepHandler)
{
    const std::chrono::nanoseconds initialStepSize{0x123456};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::TimeSyncService timeSyncService{
        mockLifecycleService};

    EXPECT_CALL(capi, SilKit_TimeSyncService_SetSimulationStepHandler(mockTimeSyncService, testing::_, testing::_,
                                                                      initialStepSize.count()));

    timeSyncService.SetSimulationStepHandler(
        [](std::chrono::nanoseconds, std::chrono::nanoseconds) {
            // do nothing
        },
        initialStepSize);
}

TEST_F(HourglassOrchestrationTest, SilKit_TimeSyncService_SetSimulationStepHandlerAsync)
{
    const std::chrono::nanoseconds initialStepSize{0x123456};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::TimeSyncService timeSyncService{
        mockLifecycleService};

    EXPECT_CALL(capi, SilKit_TimeSyncService_SetSimulationStepHandlerAsync(mockTimeSyncService, testing::_, testing::_,
                                                                           initialStepSize.count()));

    timeSyncService.SetSimulationStepHandlerAsync(
        [](std::chrono::nanoseconds, std::chrono::nanoseconds) {
            // do nothing
        },
        initialStepSize);
}

TEST_F(HourglassOrchestrationTest, SilKit_TimeSyncService_CompleteSimulationStep)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::TimeSyncService timeSyncService{
        mockLifecycleService};

    EXPECT_CALL(capi, SilKit_TimeSyncService_CompleteSimulationStep(mockTimeSyncService));

    timeSyncService.CompleteSimulationStep();
}

TEST_F(HourglassOrchestrationTest, SilKit_TimeSyncService_Now)
{
    const std::chrono::nanoseconds nanoseconds{0x123456};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::TimeSyncService timeSyncService{
        mockLifecycleService};

    EXPECT_CALL(capi, SilKit_TimeSyncService_Now(mockTimeSyncService, testing::_))
        .WillOnce(DoAll(SetArgPointee<1>(nanoseconds.count()), Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_EQ(timeSyncService.Now(), nanoseconds);
}

// SystemMonitor

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_Create)
{
    EXPECT_CALL(capi, SilKit_SystemMonitor_Create(testing::_, mockParticipant));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_GetParticipantStatus)
{
    const std::string participantName{"Participant1"};
    const std::string enterReason{"EnterReason"};
    const auto participantState{ParticipantState::Running};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    SilKit_ParticipantStatus cParticipantStatus{};
    SilKit_Struct_Init(SilKit_ParticipantStatus, cParticipantStatus);
    cParticipantStatus.participantName = participantName.c_str();
    cParticipantStatus.enterReason = enterReason.c_str();
    cParticipantStatus.participantState = static_cast<SilKit_ParticipantState>(participantState);
    cParticipantStatus.enterTime = UINT64_C(1000000000);
    cParticipantStatus.refreshTime = UINT64_C(2000000000);

    EXPECT_CALL(capi, SilKit_SystemMonitor_GetParticipantStatus(testing::_, mockSystemMonitor, StrEq(participantName)))
        .WillOnce(DoAll(SetArgPointee<0>(cParticipantStatus), Return(SilKit_ReturnCode_SUCCESS)));

    const auto participantStatus = systemMonitor.ParticipantStatus(participantName);
    EXPECT_THAT(participantStatus, CppParticipantStatusMatcher(&cParticipantStatus));
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_GetSystemState)
{
    const auto systemState{SystemState::Running};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi, SilKit_SystemMonitor_GetSystemState(testing::_, mockSystemMonitor))
        .WillOnce(
            DoAll(SetArgPointee<0>(static_cast<SilKit_SystemState>(systemState)), Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_EQ(systemMonitor.SystemState(), systemState);
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_AddSystemStateHandler_RemoveSystemStateHandler)
{
    const auto cHandlerId = static_cast<SilKit_HandlerId>(0x1234);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi, SilKit_SystemMonitor_AddSystemStateHandler(mockSystemMonitor, testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<3>(cHandlerId), Return(SilKit_ReturnCode_SUCCESS)));
    EXPECT_CALL(capi, SilKit_SystemMonitor_RemoveSystemStateHandler(mockSystemMonitor, cHandlerId));

    const auto cppHandlerId = systemMonitor.AddSystemStateHandler([](SilKit::Services::Orchestration::SystemState) {
        // do nothing
    });
    systemMonitor.RemoveSystemStateHandler(cppHandlerId);
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_AddParticipantStatusHandler_RemoveParticipantStatusHandler)
{
    const auto cHandlerId = static_cast<SilKit_HandlerId>(0x1234);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi,
                SilKit_SystemMonitor_AddParticipantStatusHandler(mockSystemMonitor, testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<3>(cHandlerId), Return(SilKit_ReturnCode_SUCCESS)));
    EXPECT_CALL(capi, SilKit_SystemMonitor_RemoveParticipantStatusHandler(mockSystemMonitor, cHandlerId));

    const auto cppHandlerId =
        systemMonitor.AddParticipantStatusHandler([](const SilKit::Services::Orchestration::ParticipantStatus&) {
            // do nothing
        });
    systemMonitor.RemoveParticipantStatusHandler(cppHandlerId);
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_SetParticipantConnectedHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi, SilKit_SystemMonitor_SetParticipantConnectedHandler(mockSystemMonitor, testing::_, testing::_));

    systemMonitor.SetParticipantConnectedHandler(
        [](const SilKit::Services::Orchestration::ParticipantConnectionInformation&) {
            // do nothing
        });
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_SetParticipantDisconnectedHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi,
                SilKit_SystemMonitor_SetParticipantDisconnectedHandler(mockSystemMonitor, testing::_, testing::_));

    systemMonitor.SetParticipantDisconnectedHandler(
        [](const SilKit::Services::Orchestration::ParticipantConnectionInformation&) {
            // do nothing
        });
}

TEST_F(HourglassOrchestrationTest, SilKit_SystemMonitor_IsParticipantConnected)
{
    const std::string participantName{"Participant1"};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Orchestration::SystemMonitor systemMonitor{
        mockParticipant};

    EXPECT_CALL(capi,
                SilKit_SystemMonitor_IsParticipantConnected(mockSystemMonitor, StrEq(participantName), testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(SilKit_True), Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_TRUE(systemMonitor.IsParticipantConnected(participantName));
}

// SystemController

TEST_F(HourglassOrchestrationTest, SilKit_Experimental_SystemController_Create)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Participant participant{mockParticipant};

    EXPECT_CALL(capi, SilKit_Experimental_SystemController_Create(testing::_, mockParticipant));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Participant::CreateSystemController(&participant);
}

TEST_F(HourglassOrchestrationTest, SilKit_Experimental_SystemController_AbortSimulation)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Experimental::Services::Orchestration::SystemController
        systemController{mockParticipant};

    EXPECT_CALL(capi, SilKit_Experimental_SystemController_AbortSimulation(mockSystemController));

    systemController.AbortSimulation();
}

TEST_F(HourglassOrchestrationTest, SilKit_Experimental_SystemController_SetWorkflowConfiguration)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Experimental::Services::Orchestration::SystemController
        systemController{mockParticipant};

    WorkflowConfiguration workflowConfiguration{};
    workflowConfiguration.requiredParticipantNames.emplace_back("Participant1");
    workflowConfiguration.requiredParticipantNames.emplace_back("Participant2");

    EXPECT_CALL(capi, SilKit_Experimental_SystemController_SetWorkflowConfiguration(
                          mockSystemController, WorkflowConfigurationMatcher(workflowConfiguration)));

    systemController.SetWorkflowConfiguration(workflowConfiguration);
}

} //namespace
