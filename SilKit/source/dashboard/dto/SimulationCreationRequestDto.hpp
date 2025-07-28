// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "SimulationConfigurationDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class SimulationCreationRequestDto : public oatpp::DTO
{
    DTO_INIT(SimulationCreationRequestDto, DTO)

    DTO_FIELD_INFO(started)
    {
        info->description = "Time when simulation started";
    }
    DTO_FIELD(UInt64, started);

    DTO_FIELD_INFO(configuration)
    {
        info->description = "Configuration of the simulation";
    }
    DTO_FIELD(Object<SimulationConfigurationDto>, configuration);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)