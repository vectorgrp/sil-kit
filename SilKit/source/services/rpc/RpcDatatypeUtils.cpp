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

#include "RpcDatatypeUtils.hpp"
#include "silkit/services/datatypes.hpp"
#include "Optional.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

bool MatchMediaType(const std::string& clientMediaType, const std::string& serverMediaType)
{
    return clientMediaType == "" || clientMediaType == serverMediaType;
}

SilKit::Util::Optional<SilKit::Services::Label> findLabel(
    std::vector<SilKit::Services::Label> dataLabels, std::string key)
{
    for (auto it : dataLabels)
    {
        if (it.key == key)
        {
            return {it};
        }
    }
    return SilKit::Util::Optional<SilKit::Services::Label>();
}

bool MatchLabels(const std::vector<SilKit::Services::MatchingLabel>& serverLabels,
                 const std::vector<SilKit::Services::Label>& clientLabels)
{
    if (serverLabels.size() == 0)
        return true; // clientLabels empty -> match

    for (auto&& kv : serverLabels)
    {
        auto optionalLabel = findLabel(clientLabels, kv.key);
        if (!optionalLabel.has_value())
        {
            if (kv.kind == SilKit::Services::MatchingLabel::Kind::Mandatory)
            {
                // mandatory labels must exist
                return false;
            }
            else if (kv.kind == SilKit::Services::MatchingLabel::Kind::Preferred)
            {
                // prefered labels that do not exist are ignored
                continue;
            }
        }
        else if (kv.value != optionalLabel.value().value) // Value does not match -> no match
        {
            return false;
        }
    }
    return true; // All of clientLabels match according to their rules -> match
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
