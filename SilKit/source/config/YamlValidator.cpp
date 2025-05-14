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

#include <stdexcept>
#include <set>
#include <deque>
#include <string>
#include <sstream>


#include "Configuration.hpp"
#include "rapidyaml.hpp"

//#include "YamlConversion.hpp"
#include "SilKitYamlHelper.hpp" // ParserContext

namespace {

using namespace SilKit::Config;

auto operator<<(std::stringstream& out, const ryml::Location& location) -> std::stringstream&
{
    out << location.line << ", " << location.col;
    return out;
}
struct ValidatingVisitor
{
    std::ostream& warnings;
    SilKit::Config::YamlValidator& v;
    std::string currentNodePath;
    std::deque<std::string> nodePaths;
    std::set<std::string> userDefinedPaths;

    bool ok{true};

    ValidatingVisitor(SilKit::Config::YamlValidator& validator, std::ostream& warnings)
        : v{validator}
        , warnings{warnings}
    {
        currentNodePath = v.DocumentRoot();
        nodePaths.push_back(currentNodePath);
    }
  
    ValidatingVisitor() = delete;
    ValidatingVisitor(const ValidatingVisitor& ) = delete;
    ValidatingVisitor& operator=(const ValidatingVisitor& ) = delete;

    bool PathIsAlreadyDefined(const std::string& path)
    {
        auto it = userDefinedPaths.insert(path);
        return !std::get<1>(it);
    }

    void push(ryml::ConstNodeRef node, ryml::id_type level)
    {
        auto typeStr = node.type_str();
        if (!node.has_key())
        {
            return;
        }

        auto nodeKey = to_string(node.key());
        auto nodePath = v.MakeName(currentNodePath, nodeKey);
        if (PathIsAlreadyDefined(nodePath))
        {
            warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodePath  << "\""
                     << " is already defined in path \"" << currentNodePath << "\"\n";
            ok &= false;
        }

        nodePaths.push_back(nodePath);
        currentNodePath = nodePaths.back();
        return;
    }
    void pop(ryml::ConstNodeRef node, ryml::id_type level)
    {
        if (node.has_key())
        {
            auto path = nodePaths.back();
            nodePaths.pop_back();
            currentNodePath = nodePaths.back();
            //std::cout << "POP " << level << " path= " << path << std::endl;
        }
    }

    void HandleKeyVal(const ryml::ConstNodeRef& node)
    {
        auto nodeName = to_string(node.key());
        auto valuePath = v.MakeName(currentNodePath, nodeName);
        if (!v.IsSchemaElement(valuePath))
        {
            if (v.IsReservedElementName(valuePath))
            {
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodeName << "\""
                         << " is not a valid sub-element of schema path \"" << currentNodePath << "\"\n";
                ok &= false;
            }
            else
            {
                // We only report error if the element is a reserved keyword
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodeName << "\""
                         << " is being ignored. It is not a sub-element of schema path \"";
            }

        }
        
    }
    void HandleSeq(const ryml::ConstNodeRef& node)
    {
    }

    void HandleVal(const ryml::ConstNodeRef& node)
    {
        auto value = to_string(node.val());
        auto valuePath = v.MakeName(currentNodePath, value);
        if (!v.IsSchemaElement(currentNodePath))
        {
            warnings << "At " << GetCurrentLocation(node) << ": Element \"" << value << "\""
                     << " is not a valid sub-element of schema path \"" << v.ParentName(currentNodePath) << "\"\n";
            ok &= false;

        }
    }

    bool operator()(const ryml::ConstNodeRef& node, ryml::id_type level) 
    {
        if ((node.has_key() && (node.is_map() || node.is_seq())) || node.is_keyval())
        {
            HandleKeyVal(node);
        }
        else if (node.is_map() || node.is_seq())
        {
            HandleSeq(node);
        }
        else if (node.is_val())
        {
            HandleVal(node);
        }
        else
        {
            auto typeStr = node.type_str();
            throw SilKit::ConfigurationError{"Unknown YAML Validation Error"};
        }
        return false;
    }

    bool IsValid() const
    {
        return ok;
    }
};
//! Recursive validation helper to iterate through the YAML document
bool ValidateDoc(ryml::ConstNodeRef& node, SilKit::Config::YamlValidator& v, std::ostream& warnings,
                 const std::string& parent)
{
    using namespace SilKit::Config;
    bool ok = true;
    std::set<std::string> declaredKeys;
    auto isAlreadyDefined = [&declaredKeys](auto keyNameToCheck) {
        auto it = declaredKeys.insert(keyNameToCheck);
        return !std::get<1>(it);
    };



    auto visitor = [](auto&& node, auto&& level) {
        auto typeStr = node.type_str();
        auto nodeStr = node.is_keyval() ? node.key() : "N/A";
        nodeStr = node.is_val() ? node.val() : nodeStr;
        std::cout << "VISIT: " << typeStr << ": " << nodeStr << std::endl;
        return false;
        };


#if DEBUG_NO_BUILD
    if (node.num_other_siblings() == 0)
    {
        std::string keyString;
        if (node.has_key())
        {
            keyString = to_string(node.key());
        }

        if (node.is_val())
        {
            auto nodeName = v.MakeName(parent, GetNodeName(node));
            if (v.IsSchemaElement(nodeName) && !v.IsSubelementOf(parent, nodeName))
            {
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << v.ElementName(nodeName) << "\""
                         << " is not a valid sub-element of schema path \"" << parent << "\"\n";
                ok &= false;
            }
            else
            {
                // This is a user-defined value, that is, a subelement
                // with no corresponding schema element as parent
            }
        }
        else if (node.is_seq())
        {
            for (auto&& child : node.cchildren())
            {
                ok &= ValidateDoc(child, v, warnings, v.MakeName(parent, keyString));
            }
        }
        else if (node.is_keyval())
        {
            // Key value pair
            auto&& value = to_string(node.val());
            auto keyName = v.MakeName(parent, keyString);
            auto valueName = v.MakeName(parent, value);
            if (isAlreadyDefined(keyName))
            {
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << v.ElementName(keyName) << "\""
                         << " is already defined in path \"" << parent << "\"\n";
                ok &= false;
            }
            // A nonempty, but invalid element name
            if (!keyName.empty() && !v.IsSchemaElement(keyName))
            {
                // Unknown elements, which are not found in the schema are only warnings
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << v.ElementName(keyName) << "\"";
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
            else if (v.HasSubelements(parent) && !v.IsSubelementOf(parent, keyName))
            {
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << v.ElementName(keyName) << "\""
                         << " is not a valid sub-element of schema path \"" << parent << "\"\n";
                ok &= false;
            }
            for (auto&& sibling : node.csiblings())
            {
                if (sibling == node)
                {
                    continue;
                }
                ok &= ValidateDoc(sibling, v, warnings, parent);
            }
            /*
            else if (!value.invalid())
            {
                ok &= ValidateDoc(value, v, warnings, newParent);
                if (value.is_map() || value.is_seq())
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
                else if (value.num_children() > 0)
                {
                    std::string newParent = parent;
                    if (!keyName.empty())
                    {
                        newParent = keyName;
                    }
                    ok &= ValidateDoc(value, v, warnings, newParent);
                }
            }
                */
        }
        else if (node.is_map())
        {
            for (auto&& child : node.cchildren())
            {
                ok &= ValidateDoc(child, v, warnings, v.MakeName(parent, keyString));
            }
        }
    }
#endif
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
    return elementName.substr(sep + 1, elementName.size());
}

bool YamlValidator::Validate(const std::string& yamlString, std::ostream& warnings)
{
    _alreadyDefinedPaths.clear();

    try
    {
        ryml::ParserOptions options{};
        options.locations(true);

        ryml::EventHandlerTree eventHandler{};
        auto parser = ryml::Parser(&eventHandler, options);
        parser.reserve_locations(100u);
        auto&& cinput = ryml::to_csubstr(yamlString);
        auto tree = ryml::parse_in_arena(&parser, cinput);

        ryml::Callbacks cb{};
        ParserContext ctx;
        ctx.parser = &parser;
        cb.m_user_data = &ctx;
        tree.callbacks(cb);

        auto root = tree.crootref();
        if (root.is_doc() && (root.is_map() || root.is_seq()))
        {
            std::string version;
            Read(version, root, "schemaVersion");
            if (!LoadSchema(version))
            {
                warnings << "Cannot load schema with SchemaVersion='" << version << "'" << "\n";
                return false;
            }
        }
        else
        {
            // The document does not specify 'SchemaVersion', we're assuming version '1'
            LoadSchema("1");
        }
        ValidatingVisitor visitor{*this, warnings};
        root.visit_stacked(visitor);
        return visitor.IsValid();
    }
    catch (const std::exception& ex)
    {
        warnings << ex.what() << "\n";
        return false;
    }
}

auto YamlValidator::ParentName(const std::string& elementName) const -> std::string
{
    auto sep = elementName.rfind(_elementSeparator);
    if (sep == elementName.npos)
    {
        throw ConfigurationError("elementName " + elementName + " has no parent");
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
    if (elementName.empty())
    {
        return parentEl;
    }
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
