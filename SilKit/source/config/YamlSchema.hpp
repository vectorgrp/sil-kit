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

#include <string>
#include <vector>
#include <initializer_list>

namespace SilKit {
namespace Config {

//! A single element in a schema tree.
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
