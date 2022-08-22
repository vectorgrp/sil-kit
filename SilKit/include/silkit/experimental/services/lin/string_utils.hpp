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

#include <iomanip>
#include <sstream>

#include "silkit/participant/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

inline std::string to_string(const Experimental::Services::Lin::LinSlaveConfiguration& linSlaveConfiguration);

inline std::ostream& operator<<(std::ostream& out, const Experimental::Services::Lin::LinSlaveConfiguration& linSlaveConfiguration);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const Experimental::Services::Lin::LinSlaveConfiguration& linSlaveConfiguration)
{
    std::stringstream out;
    out << linSlaveConfiguration;
    return out.str();
}

std::ostream& operator<<(std::ostream& out,
                         const Experimental::Services::Lin::LinSlaveConfiguration& linSlaveConfiguration)
{
    auto& respondingLinIds = linSlaveConfiguration.respondingLinIds;
    out << "Lin::LinSlaveConfiguration{respondingLinIds=[";
    if (respondingLinIds.size() > 0)
    {
        out << static_cast<uint16_t>(respondingLinIds[0]);
        for (auto i = 1u; i < respondingLinIds.size(); ++i)
        {
            out << "," << static_cast<uint16_t>(respondingLinIds[i]);
        }
    }

    out << "]}";
    return out;
}
    
} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
