// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataMessageDatatypeUtils.hpp"
#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

using namespace SilKit::Services;

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

} // namespace PubSub
} // namespace Services
} // namespace SilKit
