// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

namespace ib {
namespace cfg {

auto YamlToJson(const std::string& yamlString) -> std::string;
auto JsonToYaml(const std::string& jsonString) -> std::string;

bool Validate(const std::string& yamlString, std::ostream& warningMessages);

// Helpers to test the validation process

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
    YamlSchemaElem(const std::string& name, std::initializer_list<YamlSchemaElem> children)
        : name(name)
        , subelements(std::move(children))
    {
    }
};

//!< Validator is able to check the element <-> sub-element relations
class YamlValidator
{
public:
    YamlValidator();

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
    bool IsRootElement(const std::string& elementName);

    auto DocumentRoot() const -> std::string;
private:
    //Methods
    static const std::string _elSep;
    void UpdateIndex(const YamlSchemaElem& element, const std::string& currentParent);
private:
    // Members
    std::map<std::string /*elementName*/, YamlSchemaElem> _index;
};

} // namespace cfg
} // namespace ib
