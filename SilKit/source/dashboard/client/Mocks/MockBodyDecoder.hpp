// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock-function-mocker.h"

#include "OatppHeaders.hpp"

namespace SilKit {
namespace Dashboard {
class MockBodyDecoder : public oatpp::web::protocol::http::incoming::BodyDecoder
{
public:
    MOCK_METHOD(void, decode,
                (const oatpp::web::protocol::http::Headers&, oatpp::data::stream::InputStream*,
                 oatpp::data ::stream::WriteCallback*, oatpp::data::stream::IOStream*),
                (const, override));
    MOCK_METHOD(oatpp::async::CoroutineStarter, decodeAsync,
                (const oatpp::web::protocol::http::Headers&, const std::shared_ptr<oatpp::data::stream::InputStream>&,
                 const std::shared_ptr<oatpp::data::stream::WriteCallback>&,
                 const std::shared_ptr<oatpp::data::stream::IOStream>&),
                (const, override));
};
} // namespace Dashboard
} // namespace SilKit
