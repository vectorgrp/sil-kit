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

#include <map>
#include <string>
#include <ostream>

#include "YamlSchema.hpp"

namespace SilKit {
namespace Config {

//! YamlValidator is able to check the element <-> sub-element relations in a YAML document
class YamlValidator
{
public:
    bool Validate(const std::string& yamlString, std::ostream& warnings);
    auto ParentName(const std::string& elementName) const -> std::string;
    auto ElementName(const std::string& elementName) const -> std::string;

    //!< Create a valid element name based on two components
    auto MakeName(const std::string& parentEl, const std::string& elementName) const -> std::string;

    //!< Is the element a valid schema keyword?
    bool IsSchemaElement(const std::string& elementName) const;

    //!< Check that the element has sub-elements.
    bool HasSubelements(const std::string& elementName) const;
    
    //!< Check that elementName is a sub-element of parentName
    bool IsSubelementOf(const std::string& parentName, const std::string& elementName) const;
    
    //!< Check that elementName has the document's root as parent.
    bool IsRootElement(const std::string& elementName) const;

    //!< Check if the elementName (which might be a suffix) is a reserved element name.
    //   We want to discern unknown elements from misplaced elements in a document.
    bool IsReservedElementName(const std::string& elementName) const ;

    auto DocumentRoot() const -> std::string;

private:
    void UpdateIndex(const YamlSchemaElem& element, const std::string& currentParent);
    bool LoadSchema(std::string schemaVersion);

private:
    static const std::string _elementSeparator;
    std::map<std::string /*elementName*/, YamlSchemaElem> _index;
    YamlSchemaElem _schema;
};

} // namespace Config
} // namespace SilKit
