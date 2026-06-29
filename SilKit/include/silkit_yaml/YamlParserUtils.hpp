// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// UNSTABLE API: this auxiliary library does NOT carry the API/ABI stability
// guarantees of the main SIL Kit API (the silkit/ headers). It is provided to let
// out-of-tree projects parse their own YAML formats on their own schemata by
// reusing SIL Kit's thin rapidyaml wrapper. Pin your SIL Kit version.
// See silkit_yaml/README for details.

#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>

#include "silkit/participant/exception.hpp"

#include "rapidyaml.hpp"

namespace VSilKit {

// Format a rapidyaml parse location and message into a SilKit::ConfigurationError.
inline auto MakeConfigurationError(ryml::Location location,
                                   const std::string_view message) -> SilKit::ConfigurationError
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

    s << "line " << (location.line + 1) << " column " << location.col << ": " << message;

    return SilKit::ConfigurationError{s.str()};
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
    throw VSilKit::MakeConfigurationError(location, rapidyamlMessage);
}

} // namespace detail

// Error-handling callbacks for a ryml::Tree/Parser. The error callback nicely
// formats the location and throws SilKit::ConfigurationError. Requires the
// compile definition RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1.
inline auto GetRapidyamlCallbacks() -> ryml::Callbacks
{
    return ryml::Callbacks{nullptr, detail::RapidyamlAllocate, detail::RapidyamlFree, detail::RapidyamlError};
}

} // namespace VSilKit
