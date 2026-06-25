// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "services/metrics/MetricsDatatypes.hpp"

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"


namespace VSilKit {


struct IMsgForMetricsSender
    : SilKit::Core::ISender<MetricsUpdate>
    , SilKit::Core::IReceiver<>
{
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IMsgForMetricsSender;
} // namespace Core
} // namespace SilKit
