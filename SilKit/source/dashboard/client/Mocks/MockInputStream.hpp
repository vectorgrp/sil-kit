// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

struct MockInputStream : public oatpp::data::stream::InputStream
{
    MOCK_METHOD(void, setInputStreamIOMode, (oatpp::data::stream::IOMode), (override));

    MOCK_METHOD(oatpp::data::stream::IOMode, getInputStreamIOMode, (), (override));

    MOCK_METHOD(oatpp::data::stream::Context&, getInputStreamContext, (), (override));

    MOCK_METHOD(oatpp::v_io_size, read, (void*, v_buff_size, oatpp::async::Action&), (override));
};