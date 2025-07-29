// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MatchingLabelDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class RpcSpecDto : public oatpp::DTO
{
    DTO_INIT(RpcSpecDto, DTO)

    DTO_FIELD_INFO(functionName)
    {
        info->description = "Function name";
    }
    DTO_FIELD(String, functionName);

    DTO_FIELD_INFO(mediaType)
    {
        info->description = "Media type";
    }
    DTO_FIELD(String, mediaType);

    DTO_FIELD_INFO(labels)
    {
        info->description = "Labels";
    }
    DTO_FIELD(Vector<Object<MatchingLabelDto>>, labels);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)