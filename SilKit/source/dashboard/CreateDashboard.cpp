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

#include "CreateDashboard.hpp"

#include "Uri.hpp"

#include "ClientServerTestRunner.hpp"
#include "OatppContext.hpp"
#include "Dashboard.hpp"
#include "DashboardSystemApiController.hpp"
#include "DashboardTestComponents.hpp"


namespace SilKit {
namespace Dashboard {

auto CreateDashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                     const std::string& registryUri,
                     const std::string& dashboardUri) -> std::unique_ptr<SilKit::Dashboard::IDashboard>
{
    return std::make_unique<SilKit::Dashboard::OatppContext>(participantConfig, registryUri, dashboardUri);
}

auto RunDashboardTest(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                      const std::string& registryUri, const std::string& dashboardUri, std::function<void()> testCode,
                      uint64_t expectedSimulationsCount, std::chrono::seconds creationTimeout,
                      std::chrono::seconds updateTimeout) -> TestResult
{
    TestResult testResult{};
    oatpp::base::Environment::init();
    try
    {
        auto uri = SilKit::Core::Uri::Parse(dashboardUri);
        SilKit::Dashboard::DashboardTestComponents components{uri.Host(), uri.Port()};
        SilKit::Dashboard::ClientServerTestRunner runner;
        auto controller = SilKit::Dashboard::DashboardSystemApiController::createShared(expectedSimulationsCount,
                                                                                        creationTimeout, updateTimeout);
        runner.addController(controller);

        runner.run([testCode = std::move(testCode), participantConfig, &registryUri, controller]() {
            auto dashboard = std::make_unique<Dashboard>(participantConfig, registryUri);
            SILKIT_UNUSED_ARG(dashboard);

            testCode();
            controller->WaitSimulationsFinished();
        }, std::chrono::seconds(30));

        testResult.dataBySimulation = controller->GetData();
        testResult.allSimulationsFinished = controller->AllSimulationsFinished();
    }
    catch (oatpp::web::protocol::http::HttpError& error)
    {
        testResult.errorStatus = error.getInfo().status.code;
        testResult.errorMessage = error.getMessage();
    }
    testResult.objectCount = oatpp::base::Environment::getObjectsCount();
    oatpp::base::Environment::destroy();
    return testResult;
}

} // namespace Dashboard
} // namespace SilKit
