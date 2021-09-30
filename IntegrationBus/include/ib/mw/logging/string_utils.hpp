// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <sstream>

#include "LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

inline std::string to_string(const Level& ibAddress);
inline std::ostream& operator<<(std::ostream& out, const Level& ibAddress);
inline std::string to_string(const SourceLoc& sourceLoc);
inline std::ostream& operator<<(std::ostream& out, const SourceLoc& sourceLoc);
inline std::string to_string(const LogMsg& msg);
inline std::ostream& operator<<(std::ostream& out, const LogMsg& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(const Level& lvl)
{
    std::stringstream outStream;
    outStream << lvl;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& outStream, const Level& lvl)
{
    switch (lvl)
    {
    case Level::Trace: outStream << "Trace"; break;
    case Level::Debug: outStream << "Debug"; break;
    case Level::Info: outStream << "Info"; break;
    case Level::Warn: outStream << "Warn"; break;
    case Level::Error: outStream << "Error"; break;
    case Level::Critical: outStream << "Critical"; break;
    case Level::Off: outStream << "Off"; break;
    default: outStream << "Invalid logging::Level";
    }
    return outStream;
}

std::string to_string(const SourceLoc& sl)
{
    std::stringstream outStream;
    outStream << sl;
    return outStream.str();
}
std::ostream& operator<<(std::ostream& out, const SourceLoc& sl)
{
    return out << "SourceLoc{filename=\"" << sl.filename << "\","
        << "line=" << sl.line << ", funcname={\"" << sl.funcname << "\"}";
}

std::string to_string(const LogMsg& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}
std::ostream& operator<<(std::ostream& out, const LogMsg& msg)
{
    out << "LogMsg{logger=" << msg.logger_name
        << ", level=" << msg.level
        << ", time=" << msg.time.time_since_epoch().count()
        << ", source=" << msg.source
        << ", payload=\"" << msg.payload << "\""
        << "}";
    return out;
}
} // namespace logging
} // namespace mw
} // namespace ib