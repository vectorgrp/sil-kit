// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsSerdes.hpp"


namespace VSilKit {


inline auto operator<<(SilKit::Core::MessageBuffer& buffer, const MetricInfo& msg) -> SilKit::Core::MessageBuffer&
{
    return buffer << msg.name << msg.id << msg.kind;
}

inline auto operator>>(SilKit::Core::MessageBuffer& buffer, MetricInfo& out) -> SilKit::Core::MessageBuffer&
{
    return buffer >> out.name >> out.id >> out.kind;
}


void Serialize(SilKit::Core::MessageBuffer& buffer, const MetricsRegistration& msg)
{
    buffer << msg.metrics;
}

void Deserialize(SilKit::Core::MessageBuffer& buffer, MetricsRegistration& out)
{
    buffer >> out.metrics;
}


inline auto operator<<(SilKit::Core::MessageBuffer& buffer, const MetricData& msg) -> SilKit::Core::MessageBuffer&
{
    return buffer << msg.timestamp << msg.name << msg.kind << msg.value;
}

inline auto operator>>(SilKit::Core::MessageBuffer& buffer, MetricData& out) -> SilKit::Core::MessageBuffer&
{
    return buffer >> out.timestamp >> out.name >> out.kind >> out.value;
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
