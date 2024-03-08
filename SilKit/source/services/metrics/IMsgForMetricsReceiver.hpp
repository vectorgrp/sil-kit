// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MetricsDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"


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
