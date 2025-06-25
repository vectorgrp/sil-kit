// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT
#include "YamlParser.hpp"

namespace VSilKit {
auto ParseCapabilities(const std::string& input) -> std::vector<std::map<std::string, std::string>>
{
    std::vector<std::map<std::string, std::string>> result;
    auto&& cinput = ryml::to_csubstr(input);
    auto t = ryml::parse_in_arena(cinput);

    auto root = t.crootref();
    if (!root.is_seq())
    {
        throw SilKit::ConfigurationError{"First element in Capabilities string is not a sequence"};
    }
    if (root.has_children())
    {
        for (auto&& child : root.children())
        {
            if (!child.is_map())
            {
                throw SilKit::ConfigurationError{"Capabilities should be a sequence of map objects."};
            }
        }
    }
    root >> result;
    return result;
}

} // namespace VSilKit
