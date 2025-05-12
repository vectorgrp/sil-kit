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

#include "Optional.hpp"

#include "rapidyaml.hpp"

////////////////////////////////////////////////////////////////////////////////
// Miscellaneous SIL Kit Parsing Helper
////////////////////////////////////////////////////////////////////////////////
namespace SilKit {
namespace Config {
inline namespace v1 {
// rapidyaml utils, need to be defined in proper ADL context
namespace {
struct ParserContext
{
    ryml::Parser* parser{nullptr};
    ryml::Location currentLocation{};
    std::string currentContent;
    std::string expectedValue;
};

template <typename... Args>
auto Format(const std::string& fmt, Args&&... args)
{
    std::string buf;
    ryml::formatrs(&buf, ryml::to_csubstr(fmt), std::forward<Args>(args)...);
    return buf;
}

inline bool IsValidChild(const ryml::ConstNodeRef& node, const std::string& name)
{
    return node.is_map() && !node.find_child(ryml::to_csubstr(name)).invalid();
}

inline bool IsScalar(const ryml::ConstNodeRef& node)
{
    return node.is_val() || node.is_keyval();
}

inline bool IsMap(const ryml::ConstNodeRef& node)
{
    return node.is_map();
}

inline auto GetChildSafe(const ryml::ConstNodeRef& node, std::string name) -> ryml::ConstNodeRef
{
    if (IsValidChild(node, name))
    {
        return node;
    }
    else if (node.is_seq())
    {
        for (const auto& child : node.cchildren())
        {
            if (child.is_container() && IsValidChild(child, name))
            {
                return child;
            }
        }
    }
    return {};
}

inline void MakeMap(ryml::NodeRef* node)
{
    *node |= ryml::MAP;
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

template <typename T>
void Read(T& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    node[ryml::to_csubstr(name)] >> val;
}

template <typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
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

template <typename T, typename std::enable_if_t<!std::is_integral_v<T>, bool> = true>
void OptionalRead(T& val, const ryml::ConstNodeRef& node, const std::string& name)
{
    SetCurrentLocation(node, name);
    auto&& child = GetChildSafe(node, name);
    if (!child.invalid())
    {
        child[ryml::to_csubstr(name)] >> val;
    }
}

template <typename T>
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

template <typename T>
void Write(ryml::NodeRef* node, const std::string& name, const T& val)
{
    if (!node->is_map())
    {
        throw ConfigurationError("Parse error: trying to access child of something not a map");
    }
    node->append_child() << ryml::key(name) << val;
}

template <typename T>
void Write(ryml::NodeRef* node, const T& val)
{
    if (!node->is_val())
    {
        //throw ConfigurationError("Parse error: trying to write to something that is not a scalar");
    }
    node->set_val(ryml::to_csubstr(val));
}

template <typename T>
void OptionalWrite(const SilKit::Util::Optional<T>& val, ryml::NodeRef* node, const std::string& name)
{
    if (val.has_value())
    {
        node->append_child() << ryml::key(name) << *val;
    }
};

template <typename T>
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


template <typename T>
void NonDefaultWrite(const T& val, ryml::NodeRef* node, const std::string& name, const T& defaultValue)
{
    if (!(val == defaultValue))
    {
        node->append_child() << ryml::key(name) << val;
    }
}

template <typename ConfigT>
void OptionalRead_deprecated_alternative(ConfigT& value, const ryml::ConstNodeRef& node, const std::string& fieldName,
                                         std::initializer_list<std::string> deprecatedFieldNames)
{
    if (!(node.is_map() || node.is_seq()))
    {
        return;
    }

    auto hasChild = [&node](auto&& name) {
        auto&& c = GetChildSafe(node, name);
        return !c.invalid();
    };

    std::vector<std::string> presentDeprecatedFieldNames;
    std::copy_if(deprecatedFieldNames.begin(), deprecatedFieldNames.end(),
                 std::back_inserter(presentDeprecatedFieldNames), hasChild);

    if (IsValidChild(node, fieldName) && presentDeprecatedFieldNames.size() >= 1)
    {
        std::stringstream ss;
        ss << "The key \"" << fieldName << "\" and the deprecated alternatives";
        for (const auto& deprecatedFieldName : presentDeprecatedFieldNames)
        {
            ss << " \"" << deprecatedFieldName << "\"";
        }
        ss << " are present.";
        throw ConfigurationError{ss.str()};
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
        throw ConfigurationError{ss.str()};
    }

    OptionalRead(value, node, fieldName);
    for (const auto& deprecatedFieldName : deprecatedFieldNames)
    {
        OptionalRead(value, node, deprecatedFieldName);
    }
}
} // namespace v1
} // namespace Config
} // namespace SilKit
