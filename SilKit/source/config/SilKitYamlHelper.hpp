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

#include "rapidyaml.hpp"


// YAML-cpp serialization/deserialization for our config data types
namespace YAML {
using namespace SilKit::Config;

// Helper to parse a node as the given type or throw our ConversionError with the type's name in the error message.
template <typename T>
struct ParseTypeName
{
    static constexpr const char* Name()
    {
        return "Unknown Type";
    }
};

#define DEFINE_SILKIT_PARSE_TYPE_NAME(TYPE) \
    template <> \
    struct ParseTypeName<TYPE> \
    { \
        static constexpr const char* Name() \
        { \
            return #TYPE; \
        } \
    }

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
    template <typename SilKitDataType>
    static Node encode(const SilKitDataType& obj);
    template <typename SilKitDataType>
    static bool decode(const Node& node, SilKitDataType& obj);
};

#define DEFINE_SILKIT_CONVERT(TYPE) \
    template <> \
    struct convert<TYPE> : public Converter \
    { \
    }; \
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
// Miscellaneous SIL Kit Parsing Helper
////////////////////////////////////////////////////////////////////////////////
namespace SilKit {
namespace Config {

//XXXXXXXXXX RAPID YAML XXXXXXXXXX 
// rapidyaml utils, need to be defined in proper ADL context
namespace {
struct ParserContext
{
    ryml::Parser* parser{nullptr};
    ryml::Location currentLocation{};
    std::string currentContent;
    std::string expectedValue;
};

template<typename... Args>
auto Format(const std::string& fmt, Args&&... args)
{
  std::string buf;
  ryml::formatrs(&buf, ryml::to_csubstr(fmt), std::forward<Args>(args)...);
  return buf;
}

bool IsValidChild(const ryml::ConstNodeRef& node, const std::string& name)
{
    return !node.find_child(ryml::to_csubstr(name)).invalid();
}

bool IsScalar(const ryml::ConstNodeRef& node)
{
    return node.is_val() || node.is_keyval();
}

// for better error messages we keep track on the current node being read
void SetCurrentLocation(const ryml::ConstNodeRef& node, const std::string& name)
{
    auto&& cname = ryml::to_csubstr(name);
    auto&& ctx = reinterpret_cast<ParserContext*>(node.m_tree->callbacks().m_user_data);

    if (ctx)
    {
        ctx->expectedValue = name;
        if (node.is_map() && !node.find_child(ryml::to_csubstr(name)).invalid())
        {
            ctx->currentLocation = ctx->parser->location(node[cname]);
            auto cstr = ctx->parser->location_contents(ctx->currentLocation);
            if (cstr.size() > 1)
            {
                ctx->currentContent = std::string{cstr.data(), cstr.size() - 1};
            }
        }
    }
}
} // namespace

template<typename T>
void Read(T& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    node[ryml::to_csubstr(name)] >> val;
}

template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
void OptionalRead(T& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    auto tmp = ryml::fmt::overflow_checked(val);
    (void)node.get_if(ryml::to_csubstr(name), &tmp);
}

inline void OptionalRead(bool& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    if (IsValidChild(node, name))
    {
        node[name.c_str()] >> val;
    }
}

template<typename T, typename std::enable_if_t<!std::is_integral_v<T>, bool> = true>
void OptionalRead(T& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    if (IsValidChild(node, name))
    {
        node[name.c_str()] >> val;
    }
}

template<typename T>
void OptionalRead(SilKit::Util::Optional<T>& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    if (IsValidChild(node, name))
    {
        T tmp;
        node[name.c_str()] >> tmp;
        val = std::move(tmp); // needs a propper setter to set "has_value"
    }
}

template<typename T>
void Write(ryml::NodeRef* node, const std::string& name, const T& val)
{
    if (!node->is_map())
    {
        throw ConfigurationError("Parse error: trying to access child of something not a map");
    }
    node->append_child() << ryml::key(name) << val;
}

template<typename T>
void Write(ryml::NodeRef* node, const T& val)
{
    if (!node->is_val())
    {
        throw ConfigurationError("Parse error: trying to write to something that is not a scalar");
    }
    node->set_val(val);
}

template<typename T>
void OptionalWrite(const SilKit::Util::Optional<T>& val, ryml::NodeRef* node, const std::string& name)
{
    if (val.has_value())
    {
        node->append_child() << ryml::key(name) << *val;
    }
};

template<typename T>
void OptionalWrite(const std::vector<T>& val, ryml::NodeRef* node, const std::string& name)
{
    if (!val.empty())
    {
        node->append_child() << ryml::key(name) << val;
    }
};

inline void OptionalWrite(const std::string& val, ryml::NodeRef* node, const std::string& name)
{
    if (!val.empty())
    {
        node->append_child() << ryml::key(name) << val;
    }
};

template<typename T>
void NonDefaultWrite( const T& val, ryml::NodeRef* node, const std::string& name, const T& defaultValue)
{
    if (!(val == defaultValue))
    {
        node->append_child() << ryml::key(name) << val;
    }
}
//XXXXXXXXXX END RAPID YAML XXXXXXXXXX 


// Exception type for Bad SIL Kit internal type conversion
class ConversionError : public YAML::BadConversion
{
public:
    ConversionError(const YAML::Node& node, const std::string& message)
        : BadConversion(node.Mark())
    {
        msg = message;
    }
};

// Helper template function to convert SIL Kit data types with nice error message
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

template <typename ConfigT,
          typename std::enable_if<(std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value),
                                  bool>::type = true>
void OptionalWrite(const Util::Optional<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = value.value();
    }
}

template <typename ConfigT,
          typename std::enable_if<!(std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value),
                                  bool>::type = true>
void OptionalWrite(const SilKit::Util::Optional<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = YAML::Converter::encode(value.value());
    }
}

template <typename ConfigT,
          typename std::enable_if<!(std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value),
                                  bool>::type = true>
void OptionalWrite(const SilKit::Util::Optional<std::vector<ConfigT>>& value, YAML::Node& node,
                     const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = value.value();
    }
}

template <typename ConfigT>
void OptionalWrite(const std::vector<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.size() > 0)
    {
        node[fieldName] = value;
    }
}


template <typename ConfigT>
void OptionalRead(Util::Optional<ConfigT>& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

template <typename ConfigT>
void OptionalRead(ConfigT& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

template <typename ConfigT>
void OptionalRead_deprecated_alternative(ConfigT& value, const YAML::Node& node, const std::string& fieldName,
                                            std::initializer_list<std::string> deprecatedFieldNames)
{
    if (node.IsMap())
    {
        std::vector<std::string> presentDeprecatedFieldNames;
        std::copy_if(deprecatedFieldNames.begin(), deprecatedFieldNames.end(),
                     std::back_inserter(presentDeprecatedFieldNames),
                     [&node](const auto& deprecatedFieldName) { return node[deprecatedFieldName]; });

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

        OptionalRead(value, node, fieldName);
        for (const auto& deprecatedFieldName : deprecatedFieldNames)
        {
            OptionalRead(value, node, deprecatedFieldName);
        }
    }
}

template <typename ConfigT>
auto NonDefaultWrite(const std::vector<ConfigT>& values, YAML::Node& node, const std::string& fieldName,
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
auto NonDefaultWrite(const ConfigT& value, YAML::Node& node, const std::string& fieldName,
                        const ConfigT& defaultValue)
{
    if (!(value == defaultValue))
    {
        node[fieldName] = value;
    }
}

} // namespace Config
} // namespace SilKit
