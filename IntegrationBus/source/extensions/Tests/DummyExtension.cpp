// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "ib/version.hpp"

#include "IbExtensionMacros.hpp"
#include "DummyExtension.hpp"

// Define shared library extension for linking

IB_DECLARE_EXTENSION(
    DummyExtension,
    "Vector",
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch()
)
