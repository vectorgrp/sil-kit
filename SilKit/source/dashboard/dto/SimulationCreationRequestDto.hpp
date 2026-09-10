// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include "dashboard/dto/SimulationConfigurationDto.hpp"

namespace SilKit {
namespace Dashboard {

struct SimulationCreationRequestDto
{
    //! Time when the simulation started.
    uint64_t started{};
    SimulationConfigurationDto configuration;
};

} // namespace Dashboard
} // namespace SilKit
