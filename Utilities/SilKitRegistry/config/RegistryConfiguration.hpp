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

#pragma once

#include <string>

// Internal SIL Kit Headers
#include "Configuration.hpp"


namespace SilKitRegistry {
namespace Config {
namespace V1 {

constexpr inline auto GetSchemaVersion() -> const char*
{
    return "1";
}

struct RegistryConfiguration
{
    std::string description{""};
    SilKit::Util::Optional<std::string> listenUri;
    SilKit::Util::Optional<bool> enableDomainSockets;
    SilKit::Util::Optional<std::string> dashboardUri;
    SilKit::Config::Logging logging{};
};

} // namespace V1
} // namespace Config
} // namespace SilKitRegistry


namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration;

} // namespace Config
} // namespace SilKitRegistry
