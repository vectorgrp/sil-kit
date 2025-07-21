// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateDashboard.hpp"
#include "CreateDashboardInstance.hpp"


namespace SilKit {
namespace Dashboard {

auto CreateDashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                     const std::string& registryUri,
                     const std::string& dashboardUri) -> std::unique_ptr<SilKit::Dashboard::IDashboard>
{
    SILKIT_UNUSED_ARG(participantConfig);
    SILKIT_UNUSED_ARG(registryUri);
    SILKIT_UNUSED_ARG(dashboardUri);
    throw SilKit::SilKitError("SIL Kit Dashboard support is disabled");
}

auto RunDashboardTest(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                      const std::string& registryUri, const std::string& dashboardUri,
                      std::function<void()> testCode) -> TestResult
{
    SILKIT_UNUSED_ARG(participantConfig);
    SILKIT_UNUSED_ARG(registryUri);
    SILKIT_UNUSED_ARG(dashboardUri);
    SILKIT_UNUSED_ARG(testCode);
    throw SilKit::SilKitError("SIL Kit Dashboard support is disabled");
}

} // namespace Dashboard
} // namespace SilKit


namespace VSilKit {

auto CreateDashboardInstance() -> std::unique_ptr<IDashboardInstance>
{
    throw SilKit::SilKitError("SIL Kit Dashboard support is disabled");
}

} // namespace VSilKit
