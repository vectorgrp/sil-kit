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

#include "silkit/participant/exception.hpp"

#include "YamlParser.hpp"
#include "yaml-cpp/yaml.h"

#include "ParticipantConfiguration.hpp"

#include <iostream>
#include <iomanip>
#include <map>
#include <iterator>
#include <set>
#include <algorithm>

#include "YamlSchema.hpp"
#include "YamlValidator.hpp"

// Indentation support
#include "yaml-cpp/ostream_wrapper.h"

namespace {

// Indentation helper
class Indent {
public:
    Indent(int level)
        : _level{ level }
    {
    }
    int Level() const
    {
        return _level;
    }
    //Increase indentation level
    void Inc()
    {
        _level += _shift;
    }
    //Decrease indentation level
    void Dec()
    {
        _level -= _shift;
        if (_level < 0) _level = 0;
    }
    std::size_t Row() const
    {
        return _row;
    }
    void Row(std::size_t r)
    {
        _row = r;
    }

private:
    const int _shift{ 4 };
    int _level;
    std::size_t _row{ 0 };
};

// The ostream_wrapper keeps track of lines and write position
inline YAML::ostream_wrapper& operator<<(YAML::ostream_wrapper& out, Indent& indent)
{
    auto row = indent.Row();
    indent.Row(out.row()); // keep state
    if (out.row() != row)
    {
        // if we have a new line we indent
        for (auto i = 0; i < indent.Level(); i++)
        {
            out << ' ';
        }
    }
    return out;
}

// Ensure we can print any type that is supported by ostream::operator<<
template<typename T>
inline YAML::ostream_wrapper& operator<<(YAML::ostream_wrapper& out, const T& value)
{
    std::stringstream buf;
    buf << std::fixed << value;
    //ostream_wrapper accepts only strings:
    out << buf.str();
    return out;
}

void EmitScalar(YAML::ostream_wrapper& out, YAML::Node val)
{
    if (!val.IsScalar())
    {
        throw SilKit::SilKitError{ "YamlParser: EmitScalar on non-scalar type called" };
    }
    //XXX we should be able to query the scalar's type, instead of
    //    exception bombing our way to the final value.
    //out ;
    try {
        out << (val.as<bool>() ? "true" : "false");
    }
    catch (...)
    {
        auto emitAs = [](const auto& valStrRef, auto& output) {
            std::stringstream buf;
            buf << std::fixed << std::setprecision(10) << valStrRef;
            if ((buf >> output) && buf.eof())
            {
                return true;
            }
            return false;
        };
        const auto valStr = val.as<std::string>();
        //Warning: using the val.as<int> does truncate type to char
        int intNumber{ 0 };
        double floatNumber{ 0.0 };
        if (emitAs(valStr, intNumber)) {
            out << intNumber;
        }
        else if (emitAs(valStr, floatNumber))
        {
            out << floatNumber;
        }
        else
        {
            out << "\"" << valStr << "\"";
        }
    }
}
void EmitValidJson(YAML::ostream_wrapper& out, YAML::Node& node, 
    YAML::NodeType::value parentType = YAML::NodeType::Undefined)
{
    static Indent ind{ 0 };
    uint32_t seqNum = 0;
    const bool needComma = node.size() > 1;
    if (parentType == YAML::NodeType::Undefined)
    {
        //we're at the top level
        if (node.IsSequence())
        {
            out << "[" << "\n";
        }
        else if (node.IsMap())
        {
            out << "{" << "\n";
        }
        else
        {
            EmitScalar(out, node);
            return;
        }
        ind.Inc();
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
                out << ind <<  "\"" << kv.first.as<std::string>() << "\": ";
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
                out << ind << "[";
                EmitValidJson(out, val, YAML::NodeType::Sequence);
                out << ind << "]";
            }
            else if (val.IsMap())
            {
                out << ind << "{" << "\n";
                ind.Inc();
                EmitValidJson(out, val, YAML::NodeType::Map);
                ind.Dec();
                out << ind <<  "}";
            }
            else if (val.IsScalar())
            {
                out << ind;
                EmitScalar(out, val);
            }

        }
        if (needComma && (seqNum < (node.size()-1)))
        {
            out << ind << ", ";
            if(parentType != YAML::NodeType::Sequence)
            {
                //break lines if in object (map or toplevel)
                out << "\n"; 
            }
        }
        else
        {
            out << "\n";
        }
        seqNum++;
    }
    if (parentType == YAML::NodeType::Undefined)
    {
        //we're at the top level
        if (node.IsSequence())
        {
            out << "]" << "\n";
        }
        else if (node.IsMap())
        {
            out << "}" << "\n";
        }
    }
}

} // anonymous namespace

namespace SilKit {
namespace Config {

//! Helper to print the YAML document position
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
    std::stringstream buf;
    YAML::ostream_wrapper out{ buf };
    EmitValidJson(out, node);
    return buf.str();
}

} // namespace Config
} // namespace SilKit
