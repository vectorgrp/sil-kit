// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "services/metrics/MetricsDatatypes.hpp"

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"


namespace VSilKit {


struct IMsgForMetricsReceiver
    : SilKit::Core::ISender<>
    , SilKit::Core::IReceiver<MetricsUpdate>
{
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IMsgForMetricsReceiver;
} // namespace Core
} // namespace SilKit
