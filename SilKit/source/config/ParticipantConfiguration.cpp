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

#include "ParticipantConfiguration.hpp"

#include <string>
#include <type_traits>

namespace SilKit {
namespace Config {
inline namespace V1 {

auto Label::ToPublicApi() const -> SilKit::Services::MatchingLabel
{
    SilKit::Services::MatchingLabel result;

    result.key = key;
    result.value = value;

    switch (kind)
    {
    case Kind::Mandatory:
        result.kind = SilKit::Services::MatchingLabel::Kind::Mandatory;
        break;
    case Kind::Optional:
        result.kind = SilKit::Services::MatchingLabel::Kind::Optional;
        break;
    default:
        throw SilKit::ConfigurationError{
            "Invalid SilKit::Config::v1::MatchingLabel::Kind("
            + std::to_string(static_cast<std::underlying_type_t<Label::Kind>>(kind)) + ")"};
    }

    return result;
}


auto Label::FromPublicApi(const SilKit::Services::MatchingLabel& label) -> Label
{
    Label result;

    result.key = label.key;
    result.value = label.value;

    switch (label.kind)
    {
    case SilKit::Services::MatchingLabel::Kind::Mandatory:
        result.kind = Label::Kind::Mandatory;
        break;
    case SilKit::Services::MatchingLabel::Kind::Optional:
        result.kind = Label::Kind::Optional;
        break;
    default:
        throw SilKit::ConfigurationError{
            "Invalid SilKit::Services::MatchingLabel::Kind("
            + std::to_string(static_cast<std::underlying_type_t<SilKit::Services::MatchingLabel::Kind>>(label.kind))
            + ")"};
    }

    return result;
}


auto Label::VectorFromPublicApi(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::vector<Label>
{
    std::vector<SilKit::Config::V1::Label> result;
    std::transform(labels.begin(), labels.end(), std::back_inserter(result), Label::FromPublicApi);
    return result;
}


} // namespace V1
} // namespace Config
} // namespace SilKit
