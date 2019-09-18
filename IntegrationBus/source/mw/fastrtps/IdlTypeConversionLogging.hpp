// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/LoggingTopics.h"
#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

inline auto to_idl(Level level) -> idl::level::level_enum;
inline auto to_idl(const SourceLoc& idl) -> idl::SourceLoc;
inline auto to_idl(SourceLoc&& idl)->idl::SourceLoc;
inline auto to_idl(const LogMsg& idl) -> idl::LogMsg;
inline auto to_idl(LogMsg&& idl)->idl::LogMsg;

namespace idl {
inline auto from_idl(idl::SourceLoc&& idl) -> logging::SourceLoc;
inline auto from_idl(idl::LogMsg&& idl) -> logging::LogMsg;
namespace level {
inline auto from_idl(level_enum idl) -> logging::Level;
} // namespace level
} // namespace idl
} // namespace logging
} // namespace mw
} // namespace ib
