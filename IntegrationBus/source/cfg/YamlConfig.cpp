#include "YamlConfig.hpp"
#include "yaml-cpp/yaml.h"

#include "ib/cfg/Config.hpp"

#include <iostream>

namespace {
void EmitValidJson(std::ostream& out, YAML::Node& node, YAML::NodeType::value parentType = YAML::NodeType::Undefined)
{
    uint32_t seqNum = 0;
    const bool needComma = node.size() > 1;
    if (parentType == YAML::NodeType::Undefined)
    {
        //we're at the top level
        out << "{";
    }
    for (auto kv : node)
    {
        YAML::Node val;
        if (kv.IsDefined())
        {
            val = static_cast<YAML::Node>(kv);
        }
        else if (kv.second.IsDefined())
        {
            // might be key:value kind of node
            try {
                out << "\"" << kv.first.as<std::string>() << "\" : ";
            }
            catch (...)
            {

            }
            val = kv.second;
        }
        if (val.IsDefined())
        {

            if (val.IsSequence())
            {
                out << "[";
                EmitValidJson(out, val, YAML::NodeType::Sequence);
                out << "]";
            }
            else if (val.IsMap())
            {
                out << "{";
                EmitValidJson(out, val, YAML::NodeType::Map);
                out << "}";
            }
            else if (val.IsScalar())
            {
                //XXX we should be able to query the scalar's type, instead of
                //    exception bombing our way to the truth.
                try {
                        out << val.as<double>();
                }
                catch (...)
                {
                    try {
                        out << val.as<int64_t>();
                    }
                    catch (...) { 
                        try {
                            out << (val.as<bool>() ? "true" : "false");
                        }
                        catch (...)
                        {
                            out << "\"" << val.as<std::string>() << "\"";
                        }
                    }
                }
            }

        }
        if (needComma && (seqNum < (node.size()-1)))
        {
            out << ", ";
        }
        else
        {
            out << "\n";
        }
        seqNum++;
    }
    if (parentType == YAML::NodeType::Undefined)
    {
        out << "}\n";
    }
}

} //end anonymous namespace
namespace ib {
namespace cfg {

//Helper class to verify the Yaml doc's internal structure/ relations of elements
class CfgElem
{
public:
    //only used for document root, as anchor
    CfgElem(std::initializer_list<CfgElem> children)
        : _name(docRoot)
        , _children(std::move(children))
    {
    }
    CfgElem(const std::string& name)
        : _name(name)
    {
    }
    CfgElem(const std::string& name, std::initializer_list<CfgElem> children, bool optional = false)
        : _name(name)
        , _children(std::move(children))
        , _optional(optional)
    {

    }

    //Some predicates to test valid CfgElem relations

    // does the element have a parent named parentCandidate?
    bool IsChildOf(const std::string& elementName, const std::string& parentCandidate) const
    {
        return CheckIfValidChild(parentCandidate, _name, elementName);
    }

    // Is the element mentioned by name somewhere in docRoot?
    bool IsValidElement(const std::string& elemName) const
    {
        if (elemName == _name) return true;
        for (auto& child : _children)
        {
            if (IsValidElement(child._name)) return true;
        }
        return false;
    }

    auto DocRoot() const -> const std::string&
    {
        return docRoot;
    }
private:

    //assumes unique Element names
    CfgElem* FindElement(const std::string& elementName)
    {
        if (_name == elementName) return this;
        for (auto& child : _children)
        {
            auto* match = child.FindElement(elementName);
            if (match) return match;
        }
        return nullptr;
    }
    bool CheckIfValidChild(const std::string& parentToTest, const std::string& actualChild, const std::string& elementName) const
    {
        if (parentToTest == actualChild && _name == elementName)
        {
            return true;
        }
        for (auto& child : _children) {
            if (child.CheckIfValidChild(parentToTest, _name, elementName))
            {
                return true;
            }
        }
        return false;
    }

    bool _optional{false};
    const std::string docRoot{"DOCROOT"};
    std::string _name;
    std::vector<CfgElem> _children;
};


YamlValidator::YamlValidator()
{
    CfgElem docRoot{{
        {"ConfigVersion"},
        {"ConfigName"},
        {"Description"},
        {"SimulationSetup", {
                {"Participants", {
                    }
                },
                {"Switches", {
                    }
                },
                {"Links", {
                    }
                },
                {"NetworkSimulators", {
                    }
                },
                {"TimeSync", {
                    }
                },

            }
        },
        {"MiddlewareConfig", {
           }
        },
        {"LaunchConfigurations", {
            }
        },
    }};
    _rootElem = std::make_shared<CfgElem>(std::move(docRoot));
}

bool YamlValidator::IsTopLevelElement(const std::string& elementName) const
{
    return IsChildOf(elementName, _rootElem->DocRoot());
}
bool YamlValidator::IsChildOf(const std::string& elementName, const std::string& parentCandidate) const
{
    return _rootElem->IsChildOf(elementName, parentCandidate);
}
bool YamlValidator::IsValidElement(const std::string& elementName) const
{
    return _rootElem->IsValidElement(elementName);
}

auto YamlToJson(const std::string& yamlString) -> std::string
{
    auto doc = YAML::Load(yamlString);
    std::stringstream buffer;

    Validate(doc);
    EmitValidJson(buffer, doc);

    auto jsonString = buffer.str();
    return jsonString;
}

auto JsonToYaml(const std::string& jsonString) -> std::string
{
    auto doc = YAML::Load(jsonString);
    //use default emitter
    return YAML::Dump(doc);
}

void Validate(YAML::Node& node)
{
    ib::cfg::YamlValidator schema;
    // TODO compare node key/vals with schema predicates
}

} // namespace cfg
} // namespace ib
