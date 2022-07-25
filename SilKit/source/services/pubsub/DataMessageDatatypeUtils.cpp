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

#include "DataMessageDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs)
{
    return Util::ItemsAreEqual(lhs.data, rhs.data);
}

bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs)
{
    return ToDataMessageEvent(lhs) == ToDataMessageEvent(rhs);
}

bool MatchMediaType(const std::string& subMediaType, const std::string& pubMediaType)
{
    return subMediaType == "" || subMediaType == pubMediaType;
}

bool MatchLabels(const std::map<std::string, std::string>& subscriberLabels, const std::map<std::string, std::string>& publisherLabels)
{
    if (subscriberLabels.size() == 0)
        return true; // subscriberLabels empty -> match

    if (subscriberLabels.size() > publisherLabels.size())
        return false; // subscriberLabels more labels than outer set -> no match

    for (auto&& kv : subscriberLabels)
    {
        auto it = publisherLabels.find(kv.first);
        if (it == publisherLabels.end() || // Key not found -> no match
            (kv.second != "" && kv.second != (*it).second)) // Value does not match (and no wildcard given) -> no match
        {
            return false;
        }
    }
    return true; // All of subscriberLabels is there -> match
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
