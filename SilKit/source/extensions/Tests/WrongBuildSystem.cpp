/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

SILEXT_API SILEXT_EXTENSION_HANDLE SILEXT_CABI CreateExtension()
{
    return new WrongBuildSystem();
}

SILEXT_API void SILEXT_CABI ReleaseExtension(SILEXT_EXTENSION_HANDLE extension)
{
    auto* instance = static_cast<WrongBuildSystem *>(extension);
    delete instance;
}
