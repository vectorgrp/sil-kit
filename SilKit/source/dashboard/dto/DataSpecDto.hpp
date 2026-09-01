// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "dashboard/dto/MatchingLabelDto.hpp"

namespace SilKit {
namespace Dashboard {

struct DataSpecDto
{
    std::string topic;
    std::string mediaType;
    std::vector<MatchingLabelDto> labels;
};

} // namespace Dashboard
} // namespace SilKit
