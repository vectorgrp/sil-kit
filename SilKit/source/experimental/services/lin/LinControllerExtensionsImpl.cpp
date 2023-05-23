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
