// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"
#include "yaml-cpp/yaml.h"

#include "ib/cfg/Config.hpp"

#include <iostream>
#include <map>
#include <iterator>
#include <set>
#include <algorithm>

#include "YamlSchema.hpp"
#include "YamlValidator.hpp"

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
                //    exception bombing our way to the final value.
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


bool Validate(const std::string& yamlString, std::ostream& warningMessages)
{
    YamlValidator validator;
    return validator.Validate(yamlString, warningMessages);
}


//!< Helper to print the YAML document position
std::ostream& operator<<(std::ostream& out, const YAML::Mark& mark)
{
    if (!mark.is_null())
    {
        out << "line " << mark.line << ", column " << mark.column;
    }
    return out;
}

auto yaml_to_json(YAML::Node node) -> std::string
{
 //   std::stringstream buffer;
   // EmitValidJson(buffer, node);
   // return buffer.str();
    YAML::Emitter emitter;
    emitter.SetIndent(4);
    emitter << YAML::DoubleQuoted << YAML::Flow << YAML::Indent(2) << node;
    std::string result{ emitter.c_str() };
    result += '\n';
    return result;
}
} // namespace cfg
} // namespace ib
