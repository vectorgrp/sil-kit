// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "IoDatatypes.hpp"

namespace ib {
namespace sim {
namespace io {

inline std::string to_string(const AnalogIoMessage& msg);
inline std::string to_string(const DigitalIoMessage& msg);
inline std::string to_string(const PatternIoMessage& msg);
inline std::string to_string(const PwmIoMessage& msg);

inline std::ostream& operator<<(std::ostream& out, const AnalogIoMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const DigitalIoMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const PatternIoMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const PwmIoMessage& msg);
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(const AnalogIoMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const DigitalIoMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const PatternIoMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const PwmIoMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


std::ostream& operator<<(std::ostream& out, const AnalogIoMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "io::AnalogIoMessage{value=" << msg.value
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const DigitalIoMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "io::DigitalIoMessage{value=" << msg.value
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const PatternIoMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "io::PatternIoMessage{value="
        << util::AsHexString(msg.value).WithSeparator(" ").WithMaxLength(16)
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const PwmIoMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "io::PwmIoMessage{"
        << "freq=" << msg.value.frequency
        << "duty=" << msg.value.dutyCycle
        << " @" << timestamp.count() << "ms"
        << "}";
}




} // namespace io
} // namespace sim
} // namespace ib
