// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

#include "config/BasicYamlWriter.hpp"

#include "dashboard/dto/BulkUpdateDto.hpp"
#include "dashboard/dto/MetricsDto.hpp"
#include "dashboard/dto/SimulationCreationRequestDto.hpp"

namespace SilKit {
namespace Dashboard {

/*! Writes dashboard DTOs into a ryml tree for JSON emission.
 *
 *  Two ryml behaviours drive the design of the primitive overloads below:
 *
 *  - a value is emitted unquoted unless the node carries VALQUO or ryml itself decides to quote it,
 *    and `scalar_style_json_choose` treats anything number-like as plain. A std::string of "12345"
 *    would therefore go out as a bare JSON number, so every string is force-quoted.
 *  - a zero-length scalar emits `null` when its backing pointer is null and `""` when VALQUO is
 *    set, which is how unset-versus-empty is expressed.
 */
struct DashboardJsonWriter : VSilKit::BasicYamlWriter<DashboardJsonWriter>
{
    using BasicYamlWriter::BasicYamlWriter;
    using BasicYamlWriter::Write; // generic scalars and std::vector<T>

    // --- primitives -------------------------------------------------------------------------

    void Write(const std::string& value)
    {
        WriteQuoted(value);
    }

    void Write(std::string_view value)
    {
        WriteQuoted(value);
    }

    // Guard the paths that would silently emit an unquoted scalar.
    void Write(const char*) = delete;
    void Write(bool) = delete;

    //! Matches oatpp's OATPP_FLOAT_STRING_FORMAT, so statistic values keep their previous digits.
    void Write(double value)
    {
        char buffer[64];
        const int length = std::snprintf(buffer, sizeof buffer, "%.16g", value);
        node << ryml::csubstr{buffer, static_cast<size_t>(length < 0 ? 0 : length)};
    }

    void WriteNull()
    {
        node << nullptr;
    }

    //! Always emits the key, using JSON null when the value is absent.
    template <typename T>
    void WriteKeyValueOrNull(const std::string& name, const std::optional<T>& value)
    {
        if (value.has_value())
        {
            WriteKeyValue(name, *value);
            return;
        }
        MakeImpl(node.append_child() << ryml::key(name)).WriteNull();
    }

    // --- enums ------------------------------------------------------------------------------

    void Write(SystemState value)
    {
        Write(ToStringView(value));
    }

    void Write(ParticipantState value)
    {
        Write(ToStringView(value));
    }

    void Write(LabelKind value)
    {
        Write(ToStringView(value));
    }

    // --- DTOs -------------------------------------------------------------------------------

    void Write(const SimulationConfigurationDto& obj);
    void Write(const SimulationCreationRequestDto& obj);
    void Write(const SystemStatusDto& obj);
    void Write(const ParticipantStatusDto& obj);
    void Write(const MatchingLabelDto& obj);
    void Write(const DataSpecDto& obj);
    void Write(const RpcSpecDto& obj);
    void Write(const BulkSystemDto& obj);
    void Write(const BulkControllerDto& obj);
    void Write(const BulkDataServiceDto& obj);
    void Write(const BulkRpcServiceDto& obj);
    void Write(const BulkServiceInternalDto& obj);
    void Write(const BulkParticipantDto& obj);
    void Write(const BulkSimulationDto& obj);
    void Write(const MetricsUpdateDto& obj);

    //! One overload covers all three metric kinds.
    template <typename MetricValueT>
    void Write(const MetricDataDto<MetricValueT>& obj)
    {
        MakeMap();
        WriteKeyValue("ts", obj.ts);
        WriteKeyValue("pn", obj.pn);
        WriteKeyValue("mn", obj.mn);
        WriteKeyValue("mv", obj.mv);
    }

private:
    void WriteQuoted(std::string_view value);
};

} // namespace Dashboard
} // namespace SilKit
