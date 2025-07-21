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

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

ENUM(LabelKind, v_int32, //
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