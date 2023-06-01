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

#include "YamlValidator.hpp"

#include "yaml-cpp/yaml.h"

#include <stdexcept>
#include <set>

#include "YamlParser.hpp" // for operator<<(Mark)

namespace {

//! Recursive validation helper to iterate through the YAML document
bool ValidateDoc(YAML::Node& doc, const SilKit::Config::YamlValidator& v,
    std::ostream& warnings, const std::string& parent)
{
    using namespace SilKit::Config;
    bool ok = true;
    std::set<std::string> declaredKeys;
    auto isAlreadyDefined = [&declaredKeys](auto keyNameToCheck) {
        auto it = declaredKeys.insert(keyNameToCheck);
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
                // Anonymous container, keep parent as is
                ok &= ValidateDoc(node, v, warnings, parent);
            }
        }
        else if (node.first.IsDefined() && node.second.IsDefined())
        {
            // Key value pair
            auto& key = node.first;
            auto& value = node.second;
            auto keyName = v.MakeName(parent, key.Scalar());
            if (isAlreadyDefined(keyName))
            {
                warnings << "At " << key.Mark() << ": Element \"" << v.ElementName(keyName) << "\""
                         << " is already defined in path \"" << parent << "\"\n";
                ok &= false;
            }
            // A nonempty, but invalid element name
            if (!keyName.empty() && !v.IsSchemaElement(keyName))
            {
                // Unknown elements, which are not found in the schema are only warnings
                warnings << "At " << key.Mark() << ": Element \""
                    << v.ElementName(keyName) << "\"";
                if (v.IsReservedElementName(keyName))
                {
                    warnings << " is a reserved element name and as such"
                        << " not a sub-element of schema path \"";
                    // Misplacing a keyword is an error!
                    ok &= false;
                }
                else
                {
                    // We only report error if the element is a reserved keyword
                    warnings << " is being ignored. It is not a sub-element of schema path \"";
                }
                warnings << parent << "\"\n";
            }
            // We are not a subelement of parent
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
                // Nested sequences and maps might have no  key name
                std::string newParent = parent; // A fallback in case keyName is not given
                if (!keyName.empty())
                {
                    newParent = keyName;
                }

                if (v.HasSubelements(newParent))
                {
                    ok &= ValidateDoc(value, v, warnings, newParent);
                }
            }
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

} // anonymous namespace

namespace SilKit {
namespace Config {
  
const std::string YamlValidator::_elementSeparator{"/"};

bool YamlValidator::LoadSchema(std::string schemaVersion)
{
    if (schemaVersion == "1")
    {
        _schema = MakeYamlSchema();
    }
    else
    {
        return false;
    }
    // The root element in schema can be skipped
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
            // The document does not specify 'SchemaVersion', we're assuming version '1'
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
        throw SilKitError("Yaml Validation: elementName" + elementName + " has no parent");
    }
    else if (sep == 0)
    {
        // Special case for root lookups
        return _elementSeparator;
    }
    else
    {
        return elementName.substr(0, sep);
    }
}

auto YamlValidator::MakeName(const std::string& parentEl, const std::string& elementName) const -> std::string
{
    if (parentEl == _elementSeparator) // Special case for root lookups
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
        // Special case for root level lookups
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

} // namespace Config
} // namespace SilKit
