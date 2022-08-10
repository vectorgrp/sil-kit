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

#include "LabelMatching.hpp"
#include "Optional.hpp"

namespace SilKit {
namespace Util {

using namespace SilKit::Services;

Optional<MatchingLabel> TryFindLabelByKey(const std::string& key, const std::vector<MatchingLabel>& labels)
{
    for (auto it : labels)
    {
        if (it.key == key)
        {
            return {it};
        }
    }
    return Optional<MatchingLabel>();
}

bool LabelMatchesLabelList(MatchingLabel label, const std::vector<MatchingLabel>& labels)
{
    auto foundLabel = TryFindLabelByKey(label.key, labels);

    if (!foundLabel.has_value()) // Key not found
    {
        if (label.kind == MatchingLabel::Kind::Mandatory)
        {
            // Mandatory labels must exist
            return false;
        }
        // Optional labels that do not exist are ignored
    }
    else if (label.value != foundLabel.value().value) 
    {
       // Key found and value does not match -> no match
       return false;
    }

    return true;
}

bool MatchLabels(const std::vector<MatchingLabel>& labels1,
                 const std::vector<MatchingLabel>& labels2)
{
    // Check each labels1 against labels2 and bailout on negative match
    for (auto&& label : labels1)
    {
        if (!LabelMatchesLabelList(label, labels2))
        {
            return false;
        }
    }
    // Matching is symmetric: Do the reverse
    for (auto&& label : labels2)
    {
        if (!LabelMatchesLabelList(label, labels1))
        {
            return false;
        }
    }
    return true; // All of the labels match according to their rules -> match
}

} // namespace Util
} // namespace SilKit
