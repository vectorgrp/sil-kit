// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <algorithm>

#include "silkit/services/datatypes.hpp"
#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

/*! \brief The specification of topic, media type and labels for RpcClients and RpcServers
*/
class RpcSpec
{
private:
    std::string _functionName{};
    std::string _mediaType{};
    std::vector<SilKit::Services::MatchingLabel> _labels{};

public:
    RpcSpec() = default;

    //! Construct a RpcSpec via topic and mediaType.
    RpcSpec(std::string functionName, std::string mediaType)
        : _functionName{std::move(functionName)}
        , _mediaType{std::move(mediaType)}
    {
    }

    //! Add a given MatchingLabel.
    inline void AddLabel(const SilKit::Services::MatchingLabel& label);

    //! Add a MatchingLabel via key, value and matching kind.
    inline void AddLabel(const std::string& key, const std::string& value, SilKit::Services::MatchingLabel::Kind kind);

    //! Get the topic of the RpcSpec.
    auto FunctionName() const -> const std::string& { return _functionName; }
    //! Get the media type of the RpcSpec.
    auto MediaType() const -> const std::string& { return _mediaType; }
    //! Get the labels of the RpcSpec.
    auto Labels() const -> const std::vector<SilKit::Services::MatchingLabel>& { return _labels; }
};

void RpcSpec::AddLabel(const SilKit::Services::MatchingLabel& label)
{
    if (label.kind != MatchingLabel::Kind::Mandatory && label.kind != MatchingLabel::Kind::Optional)
    {
        throw ConfigurationError(
            "SilKit::Services::MatchingLabel must specify a valid SilKit::Services::MatchingLabel::Kind.");
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

void RpcSpec::AddLabel(const std::string& key, const std::string& value, SilKit::Services::MatchingLabel::Kind kind)
{
    AddLabel({key, value, kind});
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
