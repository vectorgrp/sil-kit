// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LabelMatching.hpp"
#include <optional>

namespace SilKit {
namespace Util {

using namespace SilKit::Services;

std::optional<MatchingLabel> TryFindLabelByKey(const std::string& key, const std::vector<MatchingLabel>& labels)
{
    for (auto it : labels)
    {
        if (it.key == key)
        {
            return {it};
        }
    }
    return std::optional<MatchingLabel>();
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
        // std::optional labels that do not exist are ignored
    }
    else if (label.value != foundLabel.value().value)
    {
        // Key found and value does not match -> no match
        return false;
    }

    return true;
}

bool MatchLabels(const std::vector<MatchingLabel>& labels1, const std::vector<MatchingLabel>& labels2)
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
