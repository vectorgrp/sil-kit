// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/lin/ILinController.hpp"

#include "LinControllerExtensionsImpl.hpp"
#include "ILinControllerExtensions.hpp"
#include "LinController.hpp"

namespace {

auto GetLinController(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Services::Lin::ILinControllerExtensions*
{
    auto linControllerExtensions = dynamic_cast<SilKit::Services::Lin::ILinControllerExtensions*>(linController);
    if (linControllerExtensions == nullptr)
    {
        throw SilKit::SilKitError("linController is not a valid SilKit::Services::Lin::ILinController*");
    }
    return linControllerExtensions;
}

} // namespace

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

auto AddLinSlaveConfigurationHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                         SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler)
    -> SilKit::Util::HandlerId
{
    return GetLinController(linController)->AddLinSlaveConfigurationHandler(handler);
}

void RemoveLinSlaveConfigurationHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                            SilKit::Util::HandlerId handlerId)
{
    return GetLinController(linController)->RemoveLinSlaveConfigurationHandler(handlerId);
}

auto GetSlaveConfigurationImpl(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration
{
    return GetLinController(linController)->GetSlaveConfiguration();
}

void InitDynamicImpl(SilKit::Services::Lin::ILinController* linController,
                     const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig)
{
    return GetLinController(linController)->InitDynamic(dynamicConfig);
}

auto AddFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                               std::function<void(SilKit::Services::Lin::ILinController*,
                                                  const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& msg)>
                                   handler) -> SilKit::Util::HandlerId
{
    return GetLinController(linController)->AddFrameHeaderHandler(handler);
}

void RemoveFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                  SilKit::Util::HandlerId handlerId)
{
    return GetLinController(linController)->RemoveFrameHeaderHandler(handlerId);
}

void SendDynamicResponseImpl(SilKit::Services::Lin::ILinController* linController,
                             const SilKit::Services::Lin::LinFrame& frame)
{
    return GetLinController(linController)->SendDynamicResponse(frame);
}

} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
