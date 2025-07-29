// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

inline std::ostream& operator<<(std::ostream& out,
                                const Experimental::Services::Lin::LinSlaveConfiguration& linSlaveConfiguration);

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
