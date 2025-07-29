// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "SilKitExtensionBase.hpp"

struct DummyExtension : public SilKit::SilKitExtensionBase
{
    int64_t _value{};
    using SilKitExtensionBase::SilKitExtensionBase;

    //some methods for testing dynamic cast across DLL boundaries
    int64_t GetDummyValue() const
    {
        return _value;
    }

    void SetDummyValue(int64_t newValue)
    {
        _value = newValue;
    }
};
