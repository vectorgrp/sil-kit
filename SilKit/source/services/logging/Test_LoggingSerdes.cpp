// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "LoggingSerdes.hpp"

TEST(Test_LoggingSerdes, LoggingSerdes)
{
    SilKit::Core::MessageBuffer buffer;
    SilKit::Services::Logging::LogMsg in, out;
    in.level = SilKit::Services::Logging::Level::Trace;
    SilKit::Services::Logging::SourceLoc loc;
    loc.filename = "somefile.txt";
    loc.funcname = "TEST(LoggingSerdes)";
    loc.line = 15;
    in.source = loc;
    in.payload = "Hello, logger!";

    Serialize(buffer, in);
    Deserialize(buffer, out);
    ASSERT_EQ(in, out);
}
