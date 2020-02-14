#include "IbExtensionApi/IbExtensionBase.hpp"
#include "IbExtensionApi/IbExtensionMacros.hpp"

// definitions for linking
struct WrongVersionExtension : public ib::extensions::IbExtensionBase
{
    using IbExtensionBase::IbExtensionBase;
};

IB_DECLARE_EXTENSION(
    WrongVersionExtension,
    "Vector",
    1,
    2,
    3
);

