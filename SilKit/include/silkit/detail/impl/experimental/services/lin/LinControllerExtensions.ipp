// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

void InitDynamic(SilKit::Services::Lin::ILinController* linController,
                 const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    cppLinController.ExperimentalInitDynamic(dynamicConfig);
}

auto AddFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController,
                           SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler)
    -> SilKit::Services::HandlerId
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    return cppLinController.ExperimentalAddFrameHeaderHandler(std::move(handler));
}

void RemoveFrameHeaderHandler(SilKit::Services::Lin::ILinController* linController,
                              SilKit::Services::HandlerId handlerId)
{
    auto& cppLinController = dynamic_cast<Impl::Services::Lin::LinController&>(*linController);

    cppLinController.ExperimentalRemoveFrameHeaderHandler(handlerId);
}

void SendDynamicResponse(SilKit::Services::Lin::ILinController* linController,
                         const SilKit::Services::Lin::LinFrame& linFrame)
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
