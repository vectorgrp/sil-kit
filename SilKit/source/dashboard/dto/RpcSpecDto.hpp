// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "dashboard/dto/MatchingLabelDto.hpp"

namespace SilKit {
namespace Dashboard {

struct RpcSpecDto
{
    std::string functionName;
    std::string mediaType;
    std::vector<MatchingLabelDto> labels;
};

} // namespace Dashboard
} // namespace SilKit
