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

#include "OatppContext.hpp"


#include "silkit/config/IParticipantConfiguration.hpp"
#include "Uri.hpp"

#include "DashboardComponents.hpp"
#include "Dashboard.hpp"

namespace SilKit {
namespace Dashboard {

OatppContext::OatppContext(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& registryUri, const std::string& dashboardUri)
{
    oatpp::base::Environment::init();
    auto uri = SilKit::Core::Uri::Parse(dashboardUri);
    DashboardComponents components{uri.Host(), uri.Port()};
    _dashboard = std::make_unique<Dashboard>(participantConfig, registryUri);
}

OatppContext::~OatppContext()
{
    _dashboard.reset();
    oatpp::base::Environment::destroy();
}

} // namespace Dashboard
} // namespace SilKit