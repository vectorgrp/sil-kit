// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

void Validate(const FlexrayClusterParameters& clusterParameters);

void Validate(const FlexrayNodeParameters& nodeParameters);

} // namespace Flexray
} // namespace Services
} // namespace SilKit
