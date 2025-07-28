// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class ServiceDto : public oatpp::DTO
{
    DTO_INIT(ServiceDto, DTO)

    DTO_FIELD_INFO(name)
    {
        info->description = "Name of the service";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(networkName)
    {
        info->description = "Name of the network";
    }
    DTO_FIELD(String, networkName);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)