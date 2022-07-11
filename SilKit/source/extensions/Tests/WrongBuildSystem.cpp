// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "silkit/SilKitVersion.hpp"

#include "SilKitExtensionBase.hpp"
#include "SilKitExtensionMacros.hpp"

// definitions for linking
struct WrongBuildSystem : public SilKit::SilKitExtensionBase
{
    using SilKitExtensionBase::SilKitExtensionBase;
};

// Manually declare entry symbol
extern "C" const SilKitExtensionDescriptor_t silkit_extension_descriptor
{
    SilKit::Version::Major(),
    SilKit::Version::Minor(),
    SilKit::Version::Patch(),
    "WrongBuildSystem",
    "Vector",
    "IncompatibleBuildHost",
    SILKIT_MAKE_BUILDINFOS(),
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
