// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

ENUM(LabelKind, v_int32,             //
     VALUE(Optional, 1, "optional"), //
     VALUE(Mandatory, 2, "mandatory"))

class MatchingLabelDto : public oatpp::DTO
{
    DTO_INIT(MatchingLabelDto, DTO)

    DTO_FIELD_INFO(key)
    {
        info->description = "Key of the label";
    }
    DTO_FIELD(String, key);

    DTO_FIELD_INFO(value)
    {
        info->description = "Value of the label";
    }
    DTO_FIELD(String, value);

    DTO_FIELD_INFO(kind)
    {
        info->description = "Kind of the label";
    }
    DTO_FIELD(Enum<LabelKind>::AsString, kind);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)