// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <vector>

#include "silkit/capi/SilKit.h"

#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace Services {
namespace Orchestration {

class SystemController : public SilKit::Experimental::Services::Orchestration::ISystemController
{
public:
    inline explicit SystemController(SilKit_Participant* participant);

    inline ~SystemController() override = default;

    inline void AbortSimulation() const override;

    inline void SetWorkflowConfiguration(
        const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration) override;

public:
    inline auto Get() const -> SilKit_Experimental_SystemController*;

private:
    SilKit_Experimental_SystemController* _systemController{nullptr};
};

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace Services {
namespace Orchestration {

inline SystemController::SystemController(SilKit_Participant* participant)
{
    const auto returnCode = SilKit_Experimental_SystemController_Create(&_systemController, participant);
    ThrowOnError(returnCode);
}

inline void SystemController::AbortSimulation() const
{
    const auto returnCode = SilKit_Experimental_SystemController_AbortSimulation(_systemController);
    ThrowOnError(returnCode);
}

inline void SystemController::SetWorkflowConfiguration(
    const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration)
{
    std::vector<const char*> requiredParticipantNames;
    for (const auto& string : workflowConfiguration.requiredParticipantNames)
    {
        requiredParticipantNames.emplace_back(string.c_str());
    }

    SilKit_StringList cRequiredParticipantNames;
    cRequiredParticipantNames.numStrings = requiredParticipantNames.size();
    cRequiredParticipantNames.strings = const_cast<char**>(requiredParticipantNames.data());

    SilKit_WorkflowConfiguration cWorkflowConfiguration;
    SilKit_Struct_Init(SilKit_WorkflowConfiguration, cWorkflowConfiguration);
    cWorkflowConfiguration.requiredParticipantNames = &cRequiredParticipantNames;

    const auto returnCode =
        SilKit_Experimental_SystemController_SetWorkflowConfiguration(_systemController, &cWorkflowConfiguration);
    ThrowOnError(returnCode);
}

inline auto SystemController::Get() const -> SilKit_Experimental_SystemController*
{
    return _systemController;
}

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
