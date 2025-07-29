// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Util {

bool MatchLabels(const std::vector<SilKit::Services::MatchingLabel>& labels1,
                 const std::vector<SilKit::Services::MatchingLabel>& labels2);

} // namespace Util
} // namespace SilKit
