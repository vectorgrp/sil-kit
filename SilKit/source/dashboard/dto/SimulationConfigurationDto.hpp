// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace SilKit {
namespace Dashboard {

struct SimulationConfigurationDto
{
    //! Connect URI of the simulation.
    std::string connectUri;
};

} // namespace Dashboard
} // namespace SilKit
