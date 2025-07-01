// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsSerdes.hpp"


namespace VSilKit {


inline auto operator<<(SilKit::Core::MessageBuffer& buffer, const MetricData& msg) -> SilKit::Core::MessageBuffer&
{
    return buffer << msg.timestamp << msg.name << msg.kind << msg.value << msg.nameList;
}

inline auto operator>>(SilKit::Core::MessageBuffer& buffer, MetricData& out) -> SilKit::Core::MessageBuffer&
{
    buffer >> out.timestamp >> out.name >> out.kind >> out.value;
    if (buffer.RemainingBytesLeft() > 0)
    {
        buffer >> out.nameList;
    }
    return buffer;
}


void Serialize(SilKit::Core::MessageBuffer& buffer, const MetricsUpdate& msg)
{
    buffer << msg.metrics;
}

void Deserialize(SilKit::Core::MessageBuffer& buffer, MetricsUpdate& out)
{
    buffer >> out.metrics;
}


} // namespace VSilKit
