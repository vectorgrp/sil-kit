// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Config.hpp"

namespace ib {
namespace cfg {

/* NOTA BENE
 * 1.) When adding new validation functions, please stick to the following pattern:
 * void Validate(const <CLASS_TO_BE_VALIDATED>&, const Config& ibConfig)
 *
 * This allows a uniform usage, and the main ib::cfg::Config can always be used as reference.

 * 2.) Also, add them to the apropriate parent Validation function. E.g.,
 * when adding a validation for Participant configs, you should call this method in
 * the validation method for the SimulationSetup
 */

void Validate(const Config& config);

void Validate(const SimulationSetup& testConfig, const Config& ibConfig);

void Validate(const TimeSync& testConfig, const Config& ibConfig);



} // namespace cfg
} // namespace ib
