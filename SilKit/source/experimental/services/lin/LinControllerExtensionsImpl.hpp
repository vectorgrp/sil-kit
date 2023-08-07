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

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <functional>

#include <cstdint>

// Forward Declarations

namespace SilKit {
namespace Services {
namespace Lin {
class ILinController;
struct LinFrame;
} // namespace Lin
} // namespace Services
} // namespace SilKit

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {
struct LinSlaveConfigurationEvent;
struct LinSlaveConfiguration;
struct LinControllerDynamicConfig;
struct LinFrameHeaderEvent;
} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit

namespace SilKit {
namespace Util {
enum struct HandlerId : std::uint64_t;
} // namespace Util
} // namespace SilKit


// Function Declarations

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

auto AddLinSlaveConfigurationHandlerImpl(
    SilKit::Services::Lin::ILinController* linController,
    std::function<void(SilKit::Services::Lin::ILinController*, const LinSlaveConfigurationEvent& msg)> handler)
    -> SilKit::Util::HandlerId;

void RemoveLinSlaveConfigurationHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                            SilKit::Util::HandlerId handlerId);

auto GetSlaveConfigurationImpl(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration;

void InitDynamicImpl(SilKit::Services::Lin::ILinController* linController,
                     const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig);

auto AddFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                               std::function<void(SilKit::Services::Lin::ILinController*,
                                                  const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& msg)>
                                   handler) -> SilKit::Util::HandlerId;

void RemoveFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                  SilKit::Util::HandlerId handlerId);

void SendDynamicResponseImpl(SilKit::Services::Lin::ILinController* linController,
                             const SilKit::Services::Lin::LinFrame& linFrame);

} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
