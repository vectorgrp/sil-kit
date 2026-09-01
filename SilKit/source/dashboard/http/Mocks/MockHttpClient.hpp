// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock.h"

#include "dashboard/http/IHttpClient.hpp"

namespace VSilKit {

class MockHttpClient : public IHttpClient
{
public:
    MOCK_METHOD(HttpResult, Post, (const std::string& path, const std::string& jsonBody), (override));
    MOCK_METHOD(void, Reset, (), (override));
    MOCK_METHOD(void, Abort, (), (override));
};

} // namespace VSilKit
