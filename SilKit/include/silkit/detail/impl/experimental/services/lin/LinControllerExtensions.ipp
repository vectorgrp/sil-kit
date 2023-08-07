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

#pragma once

#include "silkit/capi/Lin.h"

#include "silkit/detail/impl/services/lin/LinController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Services {
namespace Lin {

auto AddLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* cppILinController,
                                            SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler)
    -> SilKit::Util::HandlerId
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*cppILinController);

    return cppLinController.ExperimentalAddLinSlaveConfigurationHandler(handler);
}

void RemoveLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* cppILinController,
                                               SilKit::Util::HandlerId handlerId)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*cppILinController);

    return cppLinController.ExperimentalRemoveLinSlaveConfigurationHandler(handlerId);
}

auto GetSlaveConfiguration(SilKit::Services::Lin::ILinController* cppILinController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*cppILinController);

    return cppLinController.ExperimentalGetSlaveConfiguration();
}

void InitDynamic(SilKit::Services::Lin::ILinController* linController, const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    cppLinController.ExperimentalInitDynamic(dynamicConfig);
}

auto AddFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler) -> SilKit::Services::HandlerId
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    return cppLinController.ExperimentalAddFrameHeaderHandler(std::move(handler));
}

void RemoveFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Services::HandlerId handlerId)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    cppLinController.ExperimentalRemoveFrameHeaderHandler(handlerId);
}

void SendDynamicResponse(SilKit::Services::Lin::ILinController* linController, const SilKit::Services::Lin::LinFrame& linFrame)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    cppLinController.ExperimentalSendDynamicResponse(linFrame);
}

} // namespace Lin
} // namespace Services
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::AddLinSlaveConfigurationHandler;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::GetSlaveConfiguration;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::InitDynamic;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::AddFrameHeaderHandler;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::RemoveFrameHeaderHandler;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Lin::SendDynamicResponse;
} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
