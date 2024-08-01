// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MetricsDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"


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
