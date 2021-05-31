// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <map>
#include <string>
#include <ostream>

#include "YamlSchema.hpp"

namespace ib {
namespace cfg {

//!< YamlValidator is able to check the element <-> sub-element relations in a YAML document
class YamlValidator
{
public:
    //Ctor
    YamlValidator();

public:
    //Methods
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
    bool IsRootElement(const std::string& elementName);

    auto DocumentRoot() const -> std::string;

private:
    //Methods
    void UpdateIndex(const YamlSchemaElem& element, const std::string& currentParent);
private:
    // Members
    static const std::string _elementSeparator;
    std::map<std::string /*elementName*/, YamlSchemaElem> _index;
};


} // namespace cfg
} // namespace ib
