// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/json/DashboardJson.hpp"

namespace SilKit {
namespace Dashboard {

auto ParseSimulationCreationResponse(const std::string& body) -> std::optional<uint64_t>
{
    if (body.empty())
    {
        return std::nullopt;
    }

    try
    {
        ryml::Tree tree{VSilKit::GetRapidyamlCallbacks()};
        ryml::parse_json_in_arena(ryml::to_csubstr(body), &tree);

        const auto root = tree.crootref();
        if (!root.is_map())
        {
            return std::nullopt;
        }

        const auto id = root.find_child("id");
        if (id.invalid() || !id.has_val() || id.val().empty())
        {
            return std::nullopt;
        }

        uint64_t value{};
        auto checked = ryml::fmt::overflow_checked(value);
        if (!ryml::from_chars(id.val(), &checked))
        {
            return std::nullopt;
        }
        return value;
    }
    catch (const std::exception&)
    {
        // The ryml callbacks throw on malformed input.
        return std::nullopt;
    }
}

} // namespace Dashboard
} // namespace SilKit
