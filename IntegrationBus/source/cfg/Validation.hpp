// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace cfg {
inline namespace v4 {

/* NOTA BENE
 * 1.) When adding new validation functions, please stick to the following pattern:
 * void Validate(const <CLASS_TO_BE_VALIDATED>&, const Config& ibConfig)
 *
 * This allows a uniform usage, and the main ib::cfg::Config can always be used as reference.

 * 2.) Also, add them to the appropriate parent Validation function. E.g.,
 * when adding a validation for Participant configs, you should call this method in
 * the validation method for the SimulationSetup
 */

void Validate(const ib::cfg::v4::ParticipantConfiguration& configuration);

} // namespace v4
} // namespace cfg
} // namespace ib
