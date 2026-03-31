// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/SilKitVersion.hpp"

#include "SilKitExtensionMacros.hpp"
#include "DummyExtension.hpp"

// Define shared library extension for linking

SILKIT_DECLARE_EXTENSION(DummyExtension, "Vector", SilKit::Version::Major(), SilKit::Version::Minor(),
                         SilKit::Version::Patch())
