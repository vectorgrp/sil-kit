// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class SimulationConfigurationDto : public oatpp::DTO
{
    DTO_INIT(SimulationConfigurationDto, DTO)

    DTO_FIELD_INFO(connectUri)
    {
        info->description = "Connect URI";
    }
    DTO_FIELD(String, connectUri);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)