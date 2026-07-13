#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>

#include "silkit/participant/exception.hpp"

#include "config/YamlReader.hpp"
#include "config/YamlWriter.hpp"

#include "SilKitYaml/YamlSerdes.hpp"

#include "rapidyaml.hpp"


namespace SilKit {
namespace Config {

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//
// Thin wrappers around the generic SilKitYaml serdes templates, defaulting to the
// SilKit concrete reader/writer for the in-tree configuration types. They form
// the boundary that translates SilKitYaml::YamlError into the public
// SilKit::ConfigurationError, preserving SIL Kit's exception contract.
//////////////////////////////////////////////////////////////////////

template <typename T, typename R = VSilKit::YamlReader>
auto Deserialize(const std::string& input) -> T
{
    try
    {
        return SilKitYaml::Deserialize<T, R>(input);
    }
    catch (const std::exception& ex)
    {
        throw SilKit::ConfigurationError{ex.what()};
    }
}

template <typename T, typename W = VSilKit::YamlWriter>
auto Serialize(const T& input) -> std::string
{
    try
    {
        return SilKitYaml::Serialize<T, W>(input);
    }
    catch (const std::exception& ex)
    {
        throw SilKit::ConfigurationError{ex.what()};
    }
}

template <typename T, typename W = VSilKit::YamlWriter>
auto SerializeAsJson(const T& input) -> std::string
{
    try
    {
        return SilKitYaml::SerializeAsJson<T, W>(input);
    }
    catch (const std::exception& ex)
    {
        throw SilKit::ConfigurationError{ex.what()};
    }
}

} // namespace Config
} // namespace SilKit
