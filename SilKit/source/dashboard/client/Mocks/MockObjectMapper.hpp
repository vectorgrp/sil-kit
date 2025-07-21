// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock-function-mocker.h"

#include "OatppHeaders.hpp"

namespace SilKit {
namespace Dashboard {
class MockObjectMapper : public oatpp::data::mapping::ObjectMapper
{
public:
    explicit MockObjectMapper(const Info& info)
        : ObjectMapper(info)
    {
    }

    virtual ~MockObjectMapper() = default;

    MOCK_METHOD(void, write, (oatpp::data::stream::ConsistentOutputStream*, const oatpp::Void&), (const, override));
    MOCK_METHOD(oatpp::Void, read, (oatpp::parser::Caret&, const oatpp::Type* const), (const, override));
};
} // namespace Dashboard
} // namespace SilKit
