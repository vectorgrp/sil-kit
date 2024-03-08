// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MetricsDatatypes.hpp"

#include "MessageBuffer.hpp"


namespace VSilKit {


void Serialize(SilKit::Core::MessageBuffer& buffer, const MetricsRegistration& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, MetricsRegistration& out);

void Serialize(SilKit::Core::MessageBuffer& buffer, const MetricsUpdate& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, MetricsUpdate& out);


} // namespace VSilKit
