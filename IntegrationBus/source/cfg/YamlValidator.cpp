// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlValidator.hpp"

#include "yaml-cpp/yaml.h"

#include <stdexcept>
#include <set>

#include "YamlParser.hpp" //for operator<<(Mark)

namespace {

//!< Recursive validation helper to iterate through the YAML document
bool ValidateDoc(YAML::Node& doc, const ib::cfg::YamlValidator& v,
    std::ostream& warnings, const std::string& parent)
{
    using namespace ib::cfg;
    bool ok = true;
    std::set<std::string> declaredKeys;
    auto isAlreadyDefined = [&declaredKeys](auto keyName) {
        auto it = declaredKeys.insert(keyName);
        return !std::get<1>(it);
    };

    for (auto node : doc)
    {
        if (node.IsDefined())
        {
            if(node.IsScalar())
            { 
                auto nodeName = v.MakeName(parent, node.Scalar());
                if (v.IsSchemaElement(nodeName)
                    && !v.IsSubelementOf(parent, nodeName))
                {
                    warnings << "At " << node.Mark() << ": Element \""
                        << v.ElementName(nodeName)  << "\""
                        << " is not a valid sub-element of schema path \""
                        << parent << "\"\n";
                    ok &= false;
                }
                else
                {
                    // This is a user-defined value, that is, a subelement 
                    // with no corresponding schema element as parent
                }
            }
            else if (node.IsSequence() || node.IsMap())
            {
                //anonymous container, keep parent as is
                ok &= ValidateDoc(node, v, warnings, parent);
            }
        }
        else if (node.first.IsDefined() && node.second.IsDefined())
        {
            //key value pair
            auto& key = node.first;
            auto& value = node.second;
            auto keyName = v.MakeName(parent, key.Scalar());
            if (isAlreadyDefined(keyName))
            {
                warnings << "At " << key.Mark() << ": Element \"" << v.ElementName(keyName) << "\""
                         << " is already defined in path \"" << parent << "\"\n";
                ok &= false;
            }
            // a nonempty, but invalid element name
            if (!keyName.empty() && !v.IsSchemaElement(keyName))
            {
                //Unknown elements, which are not found in the schema are only warnings
                warnings << "At " << key.Mark() << ": Element \""
                    << v.ElementName(keyName) << "\"";
                if (v.IsReservedElementName(keyName))
                {
                    warnings << " is a reserved element name and as such"
                        << " not a sub-element of schema path \"";
                    // misplacing a keyword is an error!
                    ok &= false;
                }
                else
                {
                    // we only report error if the element is a reserved keyword
                    warnings << " is being ignored. It is not a sub-element of schema path \"";
                }
                warnings << parent << "\"\n";
            }
            // we are not a subelement of parent
            else if (v.HasSubelements(parent)
                && !v.IsSubelementOf(parent, keyName)
                )
            {
                warnings << "At " << key.Mark() << ": Element \""
                    << v.ElementName(keyName)  << "\""
                    << " is not a valid sub-element of schema path \""
                    << parent << "\"\n";
                ok &= false;
            }
            else if(value.IsMap() || value.IsSequence())
            {
                // nested sequences and maps might have no  key name
                std::string newParent = parent; //a fallback in case keyName is not given
                if (!keyName.empty())
                {
                    newParent = keyName;
                }

                if (v.HasSubelements(newParent))
                {
                    ok &= ValidateDoc(value, v, warnings, newParent);
                }
            }
            // XXX can this happen even for IsMap and IsSequence?
            else if (value.size() > 0)
            {
                std::string newParent = parent;
                if (!keyName.empty())
                {
                    newParent = keyName;
                }
                ok &= ValidateDoc(value, v, warnings, newParent);
            }
        }
    }
    return ok;
}

} //end anonymous

namespace ib {
namespace cfg {
  
const std::string YamlValidator::_elementSeparator{"/"};

bool YamlValidator::LoadSchema(std::string schemaVersion)
{
    if (schemaVersion == "1")
    {
        _schema = v1::MakeYamlSchema();
    }
    else
    {
        std::cout << "Warning: Unknown YAML schema version " << schemaVersion << std::endl;
        return false;
    }
    //the root element in schema can be skipped
    for (auto& subelement : _schema.subelements)
    {
        UpdateIndex(subelement, "");
    }
    return true;
}

void YamlValidator::UpdateIndex(const YamlSchemaElem& element, const std::string& currentParent)
{
    auto uniqueName = MakeName(currentParent, element.name);
    _index[uniqueName] = element;
    for (auto& subelement : element.subelements)
    {
        UpdateIndex(subelement, uniqueName);
    }
}

auto YamlValidator::ElementName(const std::string& elementName) const -> std::string
{
    auto sep = elementName.rfind(_elementSeparator);
    if (sep == elementName.npos || sep == elementName.size())
    {
        return {};
    }
    return elementName.substr(sep+1, elementName.size());
}

bool YamlValidator::Validate(const std::string& yamlString, std::ostream& warnings) 
{
    try {
        auto yamlDoc = YAML::Load(yamlString);
        if (yamlDoc.IsDefined() && yamlDoc.IsMap()  && yamlDoc["SchemaVersion"])
        {
            auto version = yamlDoc["SchemaVersion"].as<std::string>();
            if (!LoadSchema(version))
            {
                warnings << "Error: Cannot load schema with SchemaVersion='" << version << "'" << "\n";
                return false;
            }
        }
        else
        {
            // the document does not specify 'SchemaVersion', we're assuming version '1'
            LoadSchema("1");
        }
        return ValidateDoc(yamlDoc, *this,  warnings, DocumentRoot());
    }
    catch (const std::exception& ex) {
        warnings << "Error: caught exception: " << ex.what() << "\n";
        return false;
    }
}

auto YamlValidator::ParentName(const std::string& elementName) const -> std::string
{
    auto sep = elementName.rfind(_elementSeparator);
    if (sep == elementName.npos)
    {
        throw std::runtime_error("Yaml Validation: elementName" 
            + elementName + " has no parent");
    }
    else if (sep == 0)
    {
        //special case for root lookups
        return _elementSeparator;
    }
    else
    {
        return elementName.substr(0, sep);
    }
}

auto YamlValidator::MakeName(const std::string& parentEl, const std::string& elementName) const -> std::string
{
    if (parentEl == _elementSeparator) //special case for root lookups
    {
        return _elementSeparator + elementName;
    }
    else
    {
        return parentEl + _elementSeparator + elementName;
    }
}

bool YamlValidator::IsSchemaElement(const std::string& elementName) const
{
    return _index.count(elementName) > 0;
}

bool YamlValidator::HasSubelements(const std::string& elementName) const
{
    if (elementName == _elementSeparator)
    {
        //special case for root level lookups
        return true;
    }
    else
    {
        return _index.at(elementName).subelements.size() > 0;
    }
}

bool YamlValidator::IsSubelementOf(const std::string& parentName, const std::string& elementName) const
{
    return ParentName(elementName) == parentName;
}

bool YamlValidator::IsRootElement(const std::string& elementName) const
{
    return IsSubelementOf(_elementSeparator, elementName);
}

bool YamlValidator::IsReservedElementName(const std::string& queryElement) const
{
    const auto elementName = ElementName(queryElement);
    for (const auto& kv : _index)
    {
        const auto& elementPath = kv.first;
        auto idx = elementPath.find(elementName);
        if (idx != elementPath.npos)
        {
            return true;
        }
    }
    return false;
}

auto YamlValidator::DocumentRoot() const -> std::string
{
    return _elementSeparator;
}

} // namespace cfg
} // namespace ib
