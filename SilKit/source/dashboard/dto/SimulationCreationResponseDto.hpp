// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class SimulationCreationResponseDto : public oatpp::DTO
{
    DTO_INIT(SimulationCreationResponseDto, DTO)

    DTO_FIELD_INFO(id)
    {
        info->description = "Id";
    }
    DTO_FIELD(UInt64, id);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)