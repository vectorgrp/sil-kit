// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Dashboard {

//! Wire representation of SilKit::Services::MatchingLabel::Kind. Serialized as its name.
enum class LabelKind : int32_t
{
    Optional = 1,
    Mandatory = 2,
};

inline auto ToStringView(LabelKind kind) -> std::string_view
{
    switch (kind)
    {
    case LabelKind::Optional:
        return "optional";
    case LabelKind::Mandatory:
        return "mandatory";
    }
    throw SilKitError{"Dashboard: invalid LabelKind"};
}

struct MatchingLabelDto
{
    std::string key;
    std::string value;
    LabelKind kind{LabelKind::Optional};
};

} // namespace Dashboard
} // namespace SilKit
