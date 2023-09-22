#pragma once


#include "IIoContext.hpp"
#include "AsioSocketOptions.hpp"

#include "ILogger.hpp"

#include <memory>


namespace VSilKit {


auto MakeAsioIoContext(const AsioSocketOptions& socketOptions) -> std::unique_ptr<IIoContext>;


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::MakeAsioIoContext;
} // namespace Core
} // namespace SilKit
