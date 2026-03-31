// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/SilKitVersion.hpp"

#include "SilKitExtensionBase.hpp"
#include "SilKitExtensionMacros.hpp"

// definitions for linking
struct WrongBuildSystem : public SilKit::SilKitExtensionBase
{
    using SilKitExtensionBase::SilKitExtensionBase;
};

// Manually declare entry symbol
extern "C" const SilKitExtensionDescriptor_t silkit_extension_descriptor{
    SilKit::Version::Major(), SilKit::Version::Minor(), SilKit::Version::Patch(), "WrongBuildSystem", "Vector",
    "IncompatibleBuildHost",  SILKIT_MAKE_BUILDINFOS(),
};

SILEXT_API SILEXT_EXTENSION_HANDLE SILEXT_CABI CreateExtension()
{
    return new WrongBuildSystem();
}

SILEXT_API void SILEXT_CABI ReleaseExtension(SILEXT_EXTENSION_HANDLE extension)
{
    auto* instance = static_cast<WrongBuildSystem*>(extension);
    delete instance;
}
