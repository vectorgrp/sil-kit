/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <chrono>
#include <string>
#include <sstream>

#include "Configuration.hpp"
#include "Optional.hpp"

#include "yaml-cpp/yaml.h"

// YAML-cpp serialization/deserialization for our config data types
namespace YAML {
using namespace SilKit::Config;

// Helper to parse a node as the given type or throw our ConversionError with the type's name in the error message.
template <typename T>
struct ParseTypeName
{
    static constexpr const char* Name() { return  "Unknown Type"; }
};

#define DEFINE_SILKIT_PARSE_TYPE_NAME(TYPE) \
    template<> struct ParseTypeName<TYPE> { static constexpr const char* Name(){ return #TYPE;} }

template <typename T>
struct ParseTypeName<std::vector<T>>
{
    static constexpr const char* Name()
    {
        return ParseTypeName<T>::Name();
    }
};

// Encode/Decode implementation is provided as templated static methods, to reduce boiler plate code.
struct Converter
{
    // required for YAML::convert<T>:
    template<typename SilKitDataType>
    static Node encode(const SilKitDataType& obj);
    template<typename SilKitDataType>
    static bool decode(const Node& node, SilKitDataType& obj);
};

#define DEFINE_SILKIT_CONVERT(TYPE)\
    template<> struct convert<TYPE> : public Converter  { };\
    DEFINE_SILKIT_PARSE_TYPE_NAME(TYPE)

// Other types used for parsing, required in ConversionError for helpful error messages
DEFINE_SILKIT_PARSE_TYPE_NAME(int16_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(uint16_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(uint64_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(int64_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(int8_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(uint8_t);
DEFINE_SILKIT_PARSE_TYPE_NAME(int);
DEFINE_SILKIT_PARSE_TYPE_NAME(double);
DEFINE_SILKIT_PARSE_TYPE_NAME(bool);
DEFINE_SILKIT_PARSE_TYPE_NAME(std::vector<std::string>);
DEFINE_SILKIT_PARSE_TYPE_NAME(std::string);

} // namespace YAML

////////////////////////////////////////////////////////////////////////////////
// Miscellaneous SILKIT Parsing Helper
////////////////////////////////////////////////////////////////////////////////
namespace SilKit {
namespace Config {

// Exception type for Bad SILKIT internal type conversion
class ConversionError : public YAML::BadConversion
{
public:
    ConversionError(const YAML::Node& node, const std::string& message)
        : BadConversion(node.Mark())
    {
        msg = message;
    }
};

// Helper template function to convert SILKIT data types with nice error message
template <typename ValueT>
auto parse_as(const YAML::Node& node) -> ValueT
{
    try
    {
        return node.as<ValueT>();
    }
    catch (const ConversionError&)
    {
        //we already have a concise error message, propagate it to our caller
        throw;
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "Cannot parse as Type \"" << YAML::ParseTypeName<ValueT>::Name() << "\". Exception: \"" << ex.what()
           << "\". While parsing: " << YAML::Dump(node);
        throw ConversionError(node, ss.str());
    }
}

// Utility functions to encode/decode optional elements

template<typename ConfigT, typename std::enable_if<
    (std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value), bool>::type = true>
void optional_encode(const Util::Optional<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = value.value();
    }
}

template<typename ConfigT, typename std::enable_if<
    !(std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value), bool>::type = true>
void optional_encode(const SilKit::Util::Optional<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = YAML::Converter::encode(value.value());
    }
}

template<typename ConfigT>
void optional_encode(const std::vector<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.size() > 0)
    {
        node[fieldName] = value;
    }
}


template<typename ConfigT>
void optional_decode(Util::Optional<ConfigT>& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

template<typename ConfigT>
void optional_decode(ConfigT& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

template <typename ConfigT>
void optional_decode_deprecated_alternative(ConfigT& value, const YAML::Node& node, const std::string& fieldName,
                                            std::initializer_list<std::string> deprecatedFieldNames)
{
    if (node.IsMap())
    {
        std::vector<std::string> presentDeprecatedFieldNames;
        std::copy_if(deprecatedFieldNames.begin(), deprecatedFieldNames.end(),
                     std::back_inserter(presentDeprecatedFieldNames), [&node](const auto& deprecatedFieldName) {
                         return node[deprecatedFieldName];
                     });

        if (node[fieldName] && presentDeprecatedFieldNames.size() >= 1)
        {
            std::stringstream ss;
            ss << "The key \"" << fieldName << "\" and the deprected alternatives";
            for (const auto& deprecatedFieldName : presentDeprecatedFieldNames)
            {
                ss << " \"" << deprecatedFieldName << "\"";
            }
            ss << " are present.";
            throw ConversionError{node, ss.str()};
        }

        if (presentDeprecatedFieldNames.size() >= 2)
        {
            std::stringstream ss;
            ss << "The deprecated keys";
            for (const auto& deprecatedFieldName : presentDeprecatedFieldNames)
            {
                ss << " \"" << deprecatedFieldName << "\"";
            }
            ss << " are present.";
            throw ConversionError{node, ss.str()};
        }

        optional_decode(value, node, fieldName);
        for (const auto& deprecatedFieldName : deprecatedFieldNames)
        {
            optional_decode(value, node, deprecatedFieldName);
        }
    }
}

template <typename ConfigT>
auto non_default_encode(const std::vector<ConfigT>& values, YAML::Node& node, const std::string& fieldName,
    const std::vector<ConfigT>& defaultValue)
{
    // Only encode vectors that have members that deviate from a default-value.
    // And also ensure we only encode values that are user-defined.
    if (!values.empty() && !(values == defaultValue))
    {
        std::vector<ConfigT> userValues;
        static const ConfigT defaultObj{};
        // only encode non-default values
        for (const auto& value : values)
        {
            if (!(value == defaultObj))
            {
                userValues.push_back(value);
            }
        }
        if (userValues.size() > 0)
        {
            node[fieldName] = values;
        }
    }
}

template <typename ConfigT>
auto non_default_encode(const ConfigT& value, YAML::Node& node, const std::string& fieldName,
    const ConfigT& defaultValue)
{
    if (!(value == defaultValue))
    {
        node[fieldName] = value;
    }
}

} // namespace Config
} // namespace SilKit
