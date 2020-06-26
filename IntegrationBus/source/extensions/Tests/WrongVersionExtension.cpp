// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "IbExtensionBase.hpp"
#include "IbExtensionMacros.hpp"

// definitions for linking
struct WrongVersionExtension : public ib::extensions::IbExtensionBase
{
    using IbExtensionBase::IbExtensionBase;
};

IB_DECLARE_EXTENSION(
    WrongVersionExtension,
    "Vector",
    1,
    2,
    3
);

