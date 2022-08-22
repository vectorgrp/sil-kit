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
    auto Topic() const -> const std::string& { return _topic; }
    //! Get the media type of the PubSubSpec.
    auto MediaType() const -> const std::string& { return _mediaType; }
    //! Get the labels of the PubSubSpec.
    auto Labels() const -> const std::vector<SilKit::Services::MatchingLabel>& { return _labels; }
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
