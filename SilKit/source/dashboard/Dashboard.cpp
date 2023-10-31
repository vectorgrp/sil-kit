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

#include "Dashboard.hpp"

#include "OatppHeaders.hpp"
#include "ParticipantConfiguration.hpp"
#include "SetThreadName.hpp"
#include "Uri.hpp"

#include "DashboardRetryPolicy.hpp"
#include "SilKitToOatppMapper.hpp"

namespace SilKit {
namespace Dashboard {

Dashboard::Dashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                     const std::string& registryUri)
{
    SetParticipantConfigAndRegistryUri(participantConfig, registryUri);
    Run();
}

Dashboard::~Dashboard()
{
    _retry = false;
    ShutdownParticipantIfRunning();
    _done.wait();
}

void Dashboard::SetParticipantConfigAndRegistryUri(
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig, const std::string& registryUri)
{
    auto participantConfiguration = std::dynamic_pointer_cast<Config::ParticipantConfiguration>(participantConfig);

    SilKit::Core::Uri realRegistryUri(registryUri);
    if (!participantConfiguration->middleware.registryUri.empty())
    {
        realRegistryUri = SilKit::Core::Uri(participantConfiguration->middleware.registryUri);
    }

    if (!participantConfiguration->middleware.enableDomainSockets)
    {
        SilKit::Core::Uri originalRegistryUri(registryUri);

        if (originalRegistryUri.Host() == "0.0.0.0")
        {
            realRegistryUri = SilKit::Core::Uri("127.0.0.1", originalRegistryUri.Port());
        }
        if (originalRegistryUri.Host() == "[::]")
        {
            realRegistryUri = SilKit::Core::Uri("[::1]", originalRegistryUri.Port());
        }
    }

    _registryUri = realRegistryUri.EncodedString();
    participantConfiguration->middleware.registryUri = _registryUri;
    _participantConfig = participantConfiguration;
}

void Dashboard::Run()
{
    auto retryPolicy = std::make_shared<DashboardRetryPolicy>(3);
    _objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, connectionProvider);
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider, retryPolicy);
    _apiClient = DashboardSystemApiClient::createShared(requestExecutor, _objectMapper);
    _silKitToOatppMapper = std::make_shared<SilKitToOatppMapper>();
    InitParticipant();
    _done = std::async(std::launch::async, [this]() {
        SilKit::Util::SetThreadName("SK-Dash-Part");
        while (true)
        {
            _dashboardParticipant->Run();
            ResetParticipant();
            if (!_retry)
            {
                break;
            }
            InitParticipant();
        }
    });
    retryPolicy->AbortAllRetries();
}

void Dashboard::InitParticipant()
{
    std::lock_guard<decltype(_dashboardParticipantMx)> lock(_dashboardParticipantMx);
    _dashboardParticipant = std::make_unique<DashboardParticipant>(_participantConfig, _registryUri, _objectMapper,
                                                                   _apiClient, _silKitToOatppMapper);
}

void Dashboard::ShutdownParticipantIfRunning()
{
    std::lock_guard<decltype(_dashboardParticipantMx)> lock(_dashboardParticipantMx);
    if (_dashboardParticipant)
    {
        _dashboardParticipant->Shutdown();
    }
}

void Dashboard::ResetParticipant()
{
    std::lock_guard<decltype(_dashboardParticipantMx)> lock(_dashboardParticipantMx);
    _dashboardParticipant = nullptr;
}

} // namespace Dashboard
} // namespace SilKit
