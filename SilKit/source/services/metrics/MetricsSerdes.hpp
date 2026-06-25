// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "services/metrics/MetricsDatatypes.hpp"

#include "core/internal/MessageBuffer.hpp"


namespace VSilKit {


void Serialize(SilKit::Core::MessageBuffer& buffer, const MetricsUpdate& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, MetricsUpdate& out);


} // namespace VSilKit
