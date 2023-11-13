// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VAsioCapabilities.hpp"

#include "silkit/participant/exception.hpp"

#include "yaml-cpp/yaml.h"

#include <algorithm>
#include <exception>
#include <sstream>
#include <iomanip>


namespace SilKit {
namespace Core {

VAsioCapabilities::VAsioCapabilities(const std::string& string)
{
    Parse(string);
}

auto VAsioCapabilities::Count() const -> size_t
{
    return _capabilities.size();
}

auto VAsioCapabilities::HasCapability(const std::string& name) const -> bool
{
    return _capabilities.find(name) != _capabilities.end();
}

auto VAsioCapabilities::HasProxyMessageCapability() const -> bool
{
    return _hasProxyMessageCapability;
}

auto VAsioCapabilities::HasRequestParticipantConnectionCapability() const -> bool
{
    return _hasRequestParticipantConnectionCapability;
}

void VAsioCapabilities::AddCapability(const std::string& name)
{
    _capabilities.insert(name);
    UpdateCache();
}

auto VAsioCapabilities::ToCapabilitiesString() const -> std::string
{
    if (_capabilities.empty())
    {
        return std::string{};
    }

    bool first = true;

    std::ostringstream os;
    os << '[';
    for (const auto& item : _capabilities)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ',';
        }

        os << '{';
        os << R"("name":)";
        os << std::quoted(item);
        os << '}';
    }
    os << ']';

    return os.str();
}

void VAsioCapabilities::Parse(const std::string& string)
{
    if (string.empty())
    {
        // an empty string is an empty set of capabilities
        return;
    }

    try
    {
        // we (mis-)use the YAML parser for JSON parsing
        const auto node = YAML::Load(string);

        // the top-level node must be a sequence
        if (!node.IsSequence())
        {
            throw SilKit::TypeConversionError{"failed to parse capabilities string: top-level must be a sequence"};
        }

        for (const auto& item : node)
        {
            // each item of the top-level sequence must be a map
            if (!item.IsMap())
            {
                throw SilKit::TypeConversionError{"failed to parse capabilities string: capability must be a map"};
            }

            // each item must contain a name key which has a scalar value
            const auto name = item["name"];
            if (!name.IsScalar())
            {
                throw SilKit::TypeConversionError{"failed to parse capabilities string: name must be a scalar"};
            }

            AddCapability(name.Scalar());
        }

        UpdateCache();
    }
    catch (const std::exception& exception)
    {
        throw SilKit::TypeConversionError{std::string{"failed to parse capabilities string: "} + exception.what()};
    }
    catch (...)
    {
        throw SilKit::TypeConversionError{"failed to parse capabilities string due to unknown error"};
    }
}

void VAsioCapabilities::UpdateCache()
{
    _hasProxyMessageCapability = HasCapability(Capabilities::ProxyMessage);
    _hasRequestParticipantConnectionCapability = HasCapability(Capabilities::RequestParticipantConnection);
}

} // namespace Core
} // namespace SilKit
