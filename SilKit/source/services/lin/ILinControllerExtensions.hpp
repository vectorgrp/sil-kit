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

#include "silkit/services/lin/ILinController.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class ILinControllerExtensions
{
public:
    virtual ~ILinControllerExtensions() = default;
    virtual auto AddLinSlaveConfigurationHandler(Experimental::Services::Lin::LinSlaveConfigurationHandler handler)
        -> SilKit::Util::HandlerId = 0;
    virtual void RemoveLinSlaveConfigurationHandler(SilKit::Util::HandlerId handlerId) = 0;
    virtual auto GetSlaveConfiguration() -> Experimental::Services::Lin::LinSlaveConfiguration = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
