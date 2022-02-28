// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <chrono>
#include <string>
#include <sstream>

#include "Configuration.hpp"
#include "Optional.hpp"

#include "yaml-cpp/yaml.h"

// YAML-cpp serialization/deserialization for our config data types
namespace YAML {
using namespace ib::cfg;

// Helper to parse a node as the given type or throw our ConversionError with the type's name in the error message.
template <typename T>
struct ParseTypeName
{
    static constexpr const char* name = "Unknown Type";
};

#define DEFINE_VIB_PARSE_TYPE_NAME(TYPE) \
    template<> struct ParseTypeName<TYPE> { static constexpr const char* name = #TYPE ; }


template<typename T> struct ParseTypeName<std::vector<T>> { static constexpr const char* name = ParseTypeName<T>::name; };
// Encode/Decode implementation is provided as templated static methods, to reduce boiler plate code.
struct Converter
{
    // required for YAML::convert<T>:
    template<typename IbDataType>
    static Node encode(const IbDataType& obj);
    template<typename IbDataType>
    static bool decode(const Node& node, IbDataType& obj);
};

#define DEFINE_VIB_CONVERT(TYPE)\
    template<> struct convert<TYPE> : public Converter  { };\
    DEFINE_VIB_PARSE_TYPE_NAME(TYPE)

// Other types used for parsing, required in ConversionError for helpful error messages
DEFINE_VIB_PARSE_TYPE_NAME(int16_t);
DEFINE_VIB_PARSE_TYPE_NAME(uint16_t);
DEFINE_VIB_PARSE_TYPE_NAME(uint64_t);
DEFINE_VIB_PARSE_TYPE_NAME(int64_t);
DEFINE_VIB_PARSE_TYPE_NAME(int8_t);
DEFINE_VIB_PARSE_TYPE_NAME(uint8_t);
DEFINE_VIB_PARSE_TYPE_NAME(int);
DEFINE_VIB_PARSE_TYPE_NAME(double);
DEFINE_VIB_PARSE_TYPE_NAME(bool);
DEFINE_VIB_PARSE_TYPE_NAME(std::vector<std::string>);
DEFINE_VIB_PARSE_TYPE_NAME(std::string);

} //end YAML

////////////////////////////////////////////////////////////////////////////////
// Misc. VIB Parsing Helper
////////////////////////////////////////////////////////////////////////////////
namespace ib {
namespace cfg {

// Exception type for Bad VIB internal type conversion
class ConversionError : public YAML::BadConversion
{
public:
    ConversionError(const YAML::Node& node, const std::string& message)
        : BadConversion(node.Mark())
    {
        msg = message;
    }
};

// Helper template function to convert VIB data types with nice error message
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
        ss << "Cannot parse as Type \"" << YAML::ParseTypeName<ValueT>::name << "\". Exception: \"" << ex.what()
           << "\". While parsing: " << YAML::Dump(node);
        throw ConversionError(node, ss.str());
    }
}

// Utility functions to encode/decode optional elements

template<typename ConfigT, typename std::enable_if<
    (std::is_fundamental<ConfigT>::value || std::is_same<ConfigT, std::string>::value), bool>::type = true>
void optional_encode(const util::Optional<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = value.value();
    }
}


template<typename ConfigT>
void optional_decode(util::Optional<ConfigT>& value, const YAML::Node& node, const std::string& fieldName)
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

} // namespace cfg
} // namespace ib
