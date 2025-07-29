// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SilKitExtensionBase.hpp"
#include "SilKitExtensionMacros.hpp"

// definitions for linking
struct WrongVersionExtension : public SilKit::SilKitExtensionBase
{
    using SilKitExtensionBase::SilKitExtensionBase;
};

SILKIT_DECLARE_EXTENSION(WrongVersionExtension, "Vector", 1, 2, 3)
