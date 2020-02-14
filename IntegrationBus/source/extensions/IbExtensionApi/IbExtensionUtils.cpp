#include "IbExtensionApi/IbExtensionUtils.hpp"

#if !defined(IB_EXTENSION_OS)
#   define IB_EXTENSION_OS "UNKNOWN"
#endif

namespace ib { namespace extensions {


const char* BuildinfoSystem()
{
    return IB_EXTENSION_OS;
}

}//end namespace extensions
}//end namespace ib
