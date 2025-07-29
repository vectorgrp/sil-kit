// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <algorithm>

#include "silkit/services/datatypes.hpp"
#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

/*! \brief The specification of topic, media type and labels for DataPublishers and DataSubscribers
*/
class PubSubSpec
{
private:
    std::string _topic{};
    std::string _mediaType{};
    std::vector<SilKit::Services::MatchingLabel> _labels{};

public:
    PubSubSpec() = default;

    //! Construct a PubSubSpec via topic and mediaType.
    PubSubSpec(std::string topic, std::string mediaType)
        : _topic{std::move(topic)}
        , _mediaType{std::move(mediaType)}
    {
    }

    //! Add a given MatchingLabel.
    inline void AddLabel(const SilKit::Services::MatchingLabel& label);

    //! Add a MatchingLabel via key, value and matching kind.
    inline void AddLabel(const std::string& key, const std::string& value, SilKit::Services::MatchingLabel::Kind kind);

    //! Get the topic of the PubSubSpec.
    auto Topic() const -> const std::string&
    {
        return _topic;
    }
    //! Get the media type of the PubSubSpec.
    auto MediaType() const -> const std::string&
    {
        return _mediaType;
    }
    //! Get the labels of the PubSubSpec.
    auto Labels() const -> const std::vector<SilKit::Services::MatchingLabel>&
    {
        return _labels;
    }
};

void PubSubSpec::AddLabel(const SilKit::Services::MatchingLabel& label)
{
    if (label.kind != MatchingLabel::Kind::Mandatory && label.kind != MatchingLabel::Kind::Optional)
    {
        throw ConfigurationError(
            "SilKit::Services::MatchingLabel must specify a SilKit::Services::MatchingLabel::Kind.");
    }

    for (auto& it : _labels)
    {
        if (it.key == label.key)
        {
            it = label;
            return;
        }
    }

    _labels.push_back(label);
}

void PubSubSpec::AddLabel(const std::string& key, const std::string& value, SilKit::Services::MatchingLabel::Kind kind)
{
    AddLabel({key, value, kind});
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
