// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "ib/mw/sync/string_utils.hpp"

namespace ib {
namespace mw {
namespace sync {

inline std::string to_string(const NextSimTask& nextTask);

inline std::ostream& operator<<(std::ostream& out, const NextSimTask& nextTask);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const NextSimTask& nextTask)
{
    std::stringstream outStream;
    outStream << nextTask;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const NextSimTask& nextTask)
{
    auto tp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(nextTask.timePoint);
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(nextTask.duration);
    out << "sync::NextSimTask{tp=" << tp.count()
        << "ms, duration=" << duration.count()
        << "ms}";
    return out;
}

} // namespace sync
} // namespace mw
} // namespace ib
