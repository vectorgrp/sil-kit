// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "SilKitExtensionBase.hpp"
#include "SilKitExtensionMacros.hpp"

// definitions for linking
struct WrongVersionExtension : public SilKit::SilKitExtensionBase
{
    using SilKitExtensionBase::SilKitExtensionBase;
};

SILKIT_DECLARE_EXTENSION(
    WrongVersionExtension,
    "Vector",
    1,
    2,
    3
)

