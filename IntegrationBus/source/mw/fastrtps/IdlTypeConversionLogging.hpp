// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/LoggingTopics.h"
#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

auto to_idl(const SourceLoc& idl) -> idl::SourceLoc;
auto to_idl(SourceLoc&& idl)->idl::SourceLoc;
auto to_idl(const LogMsg& idl) -> idl::LogMsg;
auto to_idl(LogMsg&& idl)->idl::LogMsg;

namespace idl {

auto from_idl(idl::SourceLoc&& idl) -> logging::SourceLoc;
auto from_idl(idl::LogMsg&& idl) -> logging::LogMsg;
        
} // namespace idl
} // namespace logging
} // namespace mw
} // namespace ib
