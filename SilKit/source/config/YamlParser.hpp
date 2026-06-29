#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>

#include "config/YamlReader.hpp"
#include "config/YamlWriter.hpp"

#include "silkit_yaml/YamlSerdes.hpp"

#include "rapidyaml.hpp"


namespace SilKit {
namespace Config {

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//
// Thin wrappers around the generic VSilKit serdes templates, defaulting to the
// SilKit concrete reader/writer for the in-tree configuration types.
//////////////////////////////////////////////////////////////////////

template <typename T, typename R = VSilKit::YamlReader>
auto Deserialize(const std::string& input) -> T
{
    return VSilKit::Deserialize<T, R>(input);
}

template <typename T, typename W = VSilKit::YamlWriter>
auto Serialize(const T& input) -> std::string
{
    return VSilKit::Serialize<T, W>(input);
}

template <typename T, typename W = VSilKit::YamlWriter>
auto SerializeAsJson(const T& input) -> std::string
{
    return VSilKit::SerializeAsJson<T, W>(input);
}

} // namespace Config
} // namespace SilKit
