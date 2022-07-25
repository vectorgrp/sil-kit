// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/datatypes.hpp"
#include <vector>
#include <algorithm>

namespace SilKit {
namespace Services {
namespace PubSub {

/*! \brief A class containing pubsub/rpc relevant data spec information
*/
class DataPublisherSpec
{
private:
    std::string _topic{};
    std::string _mediaType{};
    std::vector<SilKit::Services::Label> _labels{};

public:
    DataPublisherSpec(){};

    DataPublisherSpec(std::string topic, std::string mediaType)
        : _topic{topic}
        , _mediaType{mediaType} {};

    inline void AddLabel(Label label);

    inline void AddLabel(std::string key, std::string value);

    auto Topic() const -> const std::string& { return _topic; };
    auto MediaType() const -> const std::string& { return _mediaType; };
    auto Labels() const -> const std::vector<SilKit::Services::Label>& { return _labels; };
};

/*! \brief A class containing pubsub/rpc relevant data spec information and how to match them
*/
class DataSubscriberSpec
{
private:
    std::string _topic{};
    std::string _mediaType{};
    std::vector<SilKit::Services::MatchingLabel> _labels{};

public:
    DataSubscriberSpec(){};

    DataSubscriberSpec(std::string topic, std::string mediaType)
        : _topic{topic}
        , _mediaType{mediaType} {};

    inline void AddLabel(SilKit::Services::MatchingLabel label);

    inline void AddLabel(std::string key, std::string value, SilKit::Services::MatchingLabel::Kind kind);

    auto Topic() const -> const std::string& { return _topic; };
    auto MediaType() const -> const std::string& { return _mediaType; };
    auto Labels() const -> const std::vector<SilKit::Services::MatchingLabel>& { return _labels; };
};

void DataPublisherSpec::AddLabel(Label label)
{
    for (auto& it : _labels)
    {
        if (it.key == label.key)
        {
            it = label;
            return;
        }
    }

    _labels.push_back(std::move(label));
}

void DataPublisherSpec::AddLabel(std::string key, std::string value)
{
    Label label{key, value};
    AddLabel(label);
}

void DataSubscriberSpec::AddLabel(SilKit::Services::MatchingLabel label)
{
    for (auto& it : _labels)
    {
        if (it.key == label.key)
        {
            it = label;
            return;
        }
    }

    _labels.push_back(std::move(label));
}

void DataSubscriberSpec::AddLabel(std::string key, std::string value, SilKit::Services::MatchingLabel::Kind kind)
{
    SilKit::Services::MatchingLabel label{key, value, kind};
    AddLabel(label);
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
