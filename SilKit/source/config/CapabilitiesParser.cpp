// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/CapabilitiesParser.hpp"

#include "silkit/participant/exception.hpp"

#include "SilKitYaml/BasicYamlReader.hpp"
#include "SilKitYaml/YamlSerdes.hpp"

namespace VSilKit {

using ValueT = std::vector<std::map<std::string, std::string>>;

namespace {
struct CapabilityReader: SilKitYaml::BasicYamlReader<CapabilityReader>
{
    using BasicYamlReader::BasicYamlReader;
    void Read(ValueT& value)
    {
        if(!IsSequence())
        {
            throw SilKitYaml::YamlError{"First element in Capabilities string is not a sequence"};
        }

        if(_node.has_children())
        {
            for(auto&& i: _node.cchildren())
            {
                auto&& parser = MakeImpl(i);
                if(!parser.IsMap())
                {
                    throw SilKitYaml::YamlError{"Capabilities should be a sequence of map objects."};
                }
                std::map<std::string, std::string> element;
                static_cast<SilKitYaml::BasicYamlReader<CapabilityReader>&>(parser).Read(element);
                value.emplace_back(std::move(element));
            }
        }
    }
};

} // end namespace
auto ParseCapabilities(const std::string& input) -> std::vector<std::map<std::string, std::string>>
{
    try {
        ValueT result;
        return SilKitYaml::Deserialize<ValueT, CapabilityReader>(input);
    } catch(const SilKitYaml::YamlError& ex)
    {
        throw SilKit::ConfigurationError{ex.what()};
    }
}

} // namespace VSilKit
