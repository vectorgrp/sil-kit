// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "silkit/version.hpp"

#include "SilKitExtensionMacros.hpp"
#include "DummyExtension.hpp"

// Define shared library extension for linking

SILKIT_DECLARE_EXTENSION(
    DummyExtension,
    "Vector",
    SilKit::Version::Major(),
    SilKit::Version::Minor(),
    SilKit::Version::Patch()
)
