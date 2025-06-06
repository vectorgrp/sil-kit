// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace VSilKit {

struct IAttributeMetric
{
    virtual ~IAttributeMetric() = default;
    virtual void Clear() = 0;
    virtual void Add(const std::string& value) = 0;
};

} // namespace VSilKit
