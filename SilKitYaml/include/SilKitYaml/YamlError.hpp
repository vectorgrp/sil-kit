// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <string>
#include <utility>

namespace SilKitYaml {

//! \brief Exception thrown by SilKitYaml on YAML parse/serialize errors.
//!
//! SilKitYaml is self-contained and depends on nothing from SIL Kit; it reports
//! all errors via this type. Hosting projects that wrap SilKitYaml (such as SIL
//! Kit itself) may translate this into their own exception type at their boundary.
class YamlError : public std::exception
{
    std::string _what;

public:
    explicit YamlError(std::string message)
        : _what{std::move(message)}
    {
    }

    explicit YamlError(const char* message)
        : _what{message}
    {
    }

    const char* what() const noexcept override
    {
        return _what.c_str();
    }
};

} // namespace SilKitYaml
