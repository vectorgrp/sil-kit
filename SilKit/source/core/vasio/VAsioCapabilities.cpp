// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "VAsioCapabilities.hpp"

#include "silkit/participant/exception.hpp"

#include "YamlParser.hpp"

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
        // the top-level node must be a sequence
        // each item of the top-level sequence must be a map
        auto caps = VSilKit::ParseCapabilities(string);
        for (auto&& item : caps)
        {
            // each item must contain a name key which has a scalar value
            const auto name = item.at("name");
            AddCapability(name);
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
