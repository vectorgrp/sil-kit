#include "IbExtensionApi/IbExtensionMacros.hpp"
#include "DummyExtension.hpp"
#include "ib/version.hpp"

// Define shared library extension for linking

IB_DECLARE_EXTENSION(
    DummyExtension,
    "Vector",
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch()
)
