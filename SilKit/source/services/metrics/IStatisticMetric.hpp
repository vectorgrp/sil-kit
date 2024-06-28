// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace VSilKit {

struct IStatisticMetric
{
    virtual ~IStatisticMetric() = default;
    virtual void Take(double value) = 0;
};

} // namespace VSilKit