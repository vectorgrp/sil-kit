// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "ib/version.hpp"

#include "IbExtensionBase.hpp"
#include "IbExtensionMacros.hpp"

// definitions for linking
struct WrongBuildSystem : public ib::extensions::IbExtensionBase
{
    using IbExtensionBase::IbExtensionBase;
};

// Manually declare entry symbol
extern "C" const IbExtensionDescriptor_t vib_extension_descriptor
{
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch(),
    "WrongBuildSystem",
    "Vector",
    "IncompatibleBuildHost",
    IB_MAKE_BUILDINFOS(),
};

VIBE_API VIBE_EXTENSION_HANDLE VIBE_CABI CreateExtension()
{
    return new WrongBuildSystem();
}

VIBE_API void VIBE_CABI ReleaseExtension(VIBE_EXTENSION_HANDLE extension)
{
    auto* instance = static_cast<WrongBuildSystem *>(extension);
    delete instance;
}
