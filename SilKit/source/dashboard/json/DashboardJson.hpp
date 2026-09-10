// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "config/YamlParserUtils.hpp"

#include "dashboard/json/DashboardJsonWriter.hpp"

namespace SilKit {
namespace Dashboard {

//! Serialize a dashboard DTO to compact JSON.
template <typename T>
auto ToJson(const T& dto) -> std::string
{
    ryml::Tree tree{VSilKit::GetRapidyamlCallbacks()};
    DashboardJsonWriter writer{tree.rootref()};
    writer.Write(dto);
    return ryml::emitrs_json<std::string>(tree);
}

/*! Extract the simulation id from a createSimulation response body, i.e. {"id":<uint64>}.
 *
 *  Returns std::nullopt for anything unusable rather than throwing: the caller degrades to "creating
 *  simulation failed", which is what the previous oatpp path reported for a non-201 response.
 *
 *  Unlike that path, unknown fields are tolerated. oatpp ran with allowUnknownFields disabled, so an
 *  extra field in the response threw out of the dashboard's worker thread and killed it.
 */
auto ParseSimulationCreationResponse(const std::string& body) -> std::optional<uint64_t>;

} // namespace Dashboard
} // namespace SilKit
