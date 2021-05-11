#include "YamlConfig.hpp"
#include "yaml-cpp/yaml.h"

#include "ib/cfg/Config.hpp"

#include <iostream>

namespace {
    void EmitValidJson(std::ostream& out, YAML::Node& node, YAML::NodeType::value parentType = YAML::NodeType::Undefined)
    {
        int seqNum = 0;
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
                    try {
                        out << val.as<int64_t>();
                    }
                    catch (...)
                    {
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

auto YamlToJson(const std::string& yamlString) -> std::string
{
    auto doc = YAML::Load(yamlString);
    std::stringstream buffer;
    EmitValidJson(buffer, doc);
    auto yamlAsJson = buffer.str();
    //YAML::Emitter emitter;
    //EmitValidJson(emitter, doc, true);//XXX not working
    // this emitter produces valid JSON, but integers are escaped as strings!
    //emitter << YAML::DoubleQuoted << YAML::Flow << doc;
    //std::string yamlAsJson = emitter.c_str();
    return yamlAsJson;
}

auto JsonToYaml(const std::string& jsonString) -> std::string
{
    auto doc = YAML::Load(jsonString);
    //use default emitter
    return YAML::Dump(doc);
}

} // namespace cfg
} // namespace ib
