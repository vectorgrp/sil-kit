// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace VSilKit {

struct MetricsUpdate;

struct IMetricsSender
{
    virtual ~IMetricsSender() = default;
    virtual void Send(const MetricsUpdate& msg) = 0;
};

} // namespace VSilKit