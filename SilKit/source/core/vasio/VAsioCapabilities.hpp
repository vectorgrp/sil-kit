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

#include <string>
#include <unordered_set>


namespace SilKit {
namespace Core {

// Protocol Capability literals
struct CapabilityLiteral
{
  std::string _value;
  explicit CapabilityLiteral(const char* capabilityString)
    : _value{ capabilityString }
  {}

  operator const char* () const 
  {
    return _value.c_str();
  }
  operator const std::string& () const
  {
    return _value;
  }
};

namespace Capabilities {
const auto ProxyMessage = CapabilityLiteral{ "proxy-message" };
const auto AutonomousSynchronous = CapabilityLiteral{ "autonomous-synchronous" };
const auto RequestParticipantConnection = CapabilityLiteral{ "request-participant-connection" };
}


class VAsioCapabilities
{
public:
    VAsioCapabilities() = default;

    explicit VAsioCapabilities(const std::string& string);

    auto Count() const -> size_t;

    auto HasCapability(const std::string& name) const -> bool;

    auto ToCapabilitiesString() const -> std::string;

    void AddCapability(const std::string& name);

private:
    void Parse(const std::string& string);

private:
    std::unordered_set<std::string> _capabilities;
};

} // namespace Core
} // namespace SilKit
