// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IbExtensionApi/IbExtensionBase.hpp"

struct DummyExtension : public ib::extensions::IbExtensionBase
{
    int64_t _value{};
    using IbExtensionBase::IbExtensionBase;

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
