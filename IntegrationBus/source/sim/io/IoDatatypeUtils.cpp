// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IoDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace io { 

bool operator==(const AnalogIoMessage& lhs, const AnalogIoMessage& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.value == rhs.value;
}
    
bool operator==(const DigitalIoMessage& lhs, const DigitalIoMessage& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.value == rhs.value;
}

bool operator==(const PatternIoMessage& lhs, const PatternIoMessage& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.value == rhs.value;
}

bool operator==(const PwmValue& lhs, const PwmValue& rhs)
{
    return lhs.frequency == rhs.frequency
        && lhs.dutyCycle == rhs.dutyCycle;
}

bool operator==(const PwmIoMessage& lhs, const PwmIoMessage& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.value == rhs.value;
}


} // namespace io
} // namespace sim
} // namespace ib
