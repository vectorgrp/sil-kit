// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace VSilKit {

struct IMetricsTimerThread
{
    virtual ~IMetricsTimerThread() = default;
    virtual void Start() = 0;
};

} // namespace VSilKit

namespace SilKit {
namespace Core {
using VSilKit::IMetricsTimerThread;
} // namespace Core
} // namespace SilKit
