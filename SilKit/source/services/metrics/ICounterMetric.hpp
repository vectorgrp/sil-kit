// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

namespace VSilKit {

struct ICounterMetric
{
    virtual ~ICounterMetric() = default;
    virtual void Add(std::uint64_t delta) = 0;
    virtual void Set(std::uint64_t value) = 0;
};

} // namespace VSilKit