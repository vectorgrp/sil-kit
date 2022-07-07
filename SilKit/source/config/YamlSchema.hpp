// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <initializer_list>

namespace SilKit {
namespace Config {

//!< A single element in a schema tree.
// TODO might be extended to support type hints for parsing
struct YamlSchemaElem
{
    std::string name;
    std::vector<YamlSchemaElem> subelements;

    YamlSchemaElem() = default; //for std::map
    //CTors: allow for easy creation of a tree
    YamlSchemaElem(std::initializer_list<YamlSchemaElem> children)
        : subelements(std::move(children))
    {
    }
    // Single named element, with no sub-elements
    YamlSchemaElem(const std::string& elementName)
        : name(elementName)
    {
    }
    // A named element with sub-elements
    YamlSchemaElem(const std::string& elementName, std::initializer_list<YamlSchemaElem> children)
        : name(elementName)
        , subelements(std::move(children))
    {
    }
};

inline namespace v1 {
auto MakeYamlSchema() -> YamlSchemaElem;
} // inline namespace v1

} // namespace Config
} // namespace SilKit
