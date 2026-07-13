// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// SilKitYaml: a self-contained, header-only YAML (de)serialization layer built on
// top of rapidyaml. It is consumed internally by SIL Kit and may be vendored into
// other projects via subrepo/submodule (add_subdirectory). It depends only on a
// `rapidyaml` target. See SilKitYaml/README.md.

#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>

#include "rapidyaml.hpp"

#include "SilKitYaml/YamlError.hpp"

namespace SilKitYaml {

// Format a rapidyaml parse location and message into a SilKitYaml::YamlError.
inline auto MakeError(ryml::Location location, const std::string_view message) -> YamlError
{
    std::ostringstream s;

    s << "error parsing yaml";
    if (location.name.empty())
    {
        s << " string: ";
    }
    else
    {
        s << " file " << location.name << ": ";
    }

    s << "line " << (location.line + 1) << " column " << location.col << ": " << message;

    return YamlError{s.str()};
}

namespace detail {

inline auto RapidyamlAllocate(const size_t length, void* /*hint*/, void* /*userData*/) -> void*
{
    return std::malloc(length);
}

inline void RapidyamlFree(void* ptr, size_t /*length*/, void* /*userData*/)
{
    std::free(ptr);
}

inline void RapidyamlError(const char* message, const size_t length, ryml::Location location, void* /*userData*/)
{
    const std::string_view rapidyamlMessage{message, length};
    throw SilKitYaml::MakeError(location, rapidyamlMessage);
}

} // namespace detail

// Error-handling callbacks for a ryml::Tree/Parser. The error callback nicely
// formats the location and throws SilKitYaml::YamlError. Requires the compile
// definition RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1 (provided by the rapidyaml target).
inline auto GetRapidyamlCallbacks() -> ryml::Callbacks
{
    return ryml::Callbacks{nullptr, detail::RapidyamlAllocate, detail::RapidyamlFree, detail::RapidyamlError};
}

} // namespace SilKitYaml
