// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/lin/ILinController.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class ILinControllerExtensions
{
public:
    virtual ~ILinControllerExtensions() = default;

    virtual auto AddLinSlaveConfigurationHandler(
        SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler) -> SilKit::Util::HandlerId = 0;

    virtual void RemoveLinSlaveConfigurationHandler(SilKit::Util::HandlerId handlerId) = 0;

    virtual auto GetSlaveConfiguration() -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration = 0;

    virtual void InitDynamic(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig) = 0;

    virtual auto AddFrameHeaderHandler(SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler)
        -> SilKit::Util::HandlerId = 0;

    virtual void RemoveFrameHeaderHandler(SilKit::Util::HandlerId handlerId) = 0;

    virtual void SendDynamicResponse(const SilKit::Services::Lin::LinFrame& frame) = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
