// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "YamlParserUtils.hpp"

#include "silkit/participant/exception.hpp"

#include <sstream>


namespace {

auto RapidyamlAllocate(size_t length, void* hint, void* userData) -> void*;
void RapidyamlFree(void* ptr, size_t length, void* userData);
void RapidyamlError(const char* message, size_t length, ryml::Location location, void* userData);

} // namespace


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

auto MakeConfigurationError(ryml::Location location, const std::string_view message) -> SilKit::ConfigurationError
{
    std::ostringstream s;

    s << "error parsing configuration";
    if (location.name.empty())
    {
        s << " string: ";
    }
    else
    {
        s << " file " << location.name << ": ";
    }

    // NB: The line number seems off-by-one. This might be a bug in rapidyaml, or a misunderstanding on my end.
    s << "line " << (location.line + 1) << " column " << location.col << ": " << message;

    return SilKit::ConfigurationError{s.str()};
}

auto GetRapidyamlCallbacks() -> ryml::Callbacks
{
    return ryml::Callbacks{nullptr, RapidyamlAllocate, RapidyamlFree, RapidyamlError};
}

} // namespace VSilKit


namespace {

auto RapidyamlAllocate(const size_t length, void* /*hint*/, void* /*userData*/) -> void*
{
    return ::operator new(length);
}

void RapidyamlFree(void* ptr, size_t /*length*/, void* /*userData*/)
{
    ::operator delete(ptr);
}

void RapidyamlError(const char* message, const size_t length, ryml::Location location, void* /*userData*/)
{
    const std::string_view rapidyamlMessage{message, length};
    throw VSilKit::MakeConfigurationError(location, message);
}

} // namespace
