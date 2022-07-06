// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gtest/gtest.h"

#include "LoggingSerdes.hpp"

TEST(MwLoggingSerdes, LoggingSerdes)
{
    SilKit::Core::MessageBuffer buffer;
    SilKit::Core::Logging::LogMsg in, out;
    in.level = SilKit::Core::Logging::Level::Trace;
    SilKit::Core::Logging::SourceLoc loc;
    loc.filename = "somefile.txt";
    loc.funcname ="TEST(LoggingSerdes)";
    loc.line = 15;
    in.source = loc;
    in.payload = "Hello, logger!";

    Serialize(buffer, in);
    Deserialize(buffer, out);
    ASSERT_EQ(in, out);
}
