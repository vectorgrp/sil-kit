// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gtest/gtest.h"

#include "LoggingSerdes.hpp"

TEST(MwLoggingSerdes, LoggingSerdes)
{
    ib::mw::MessageBuffer buffer;
    ib::mw::logging::LogMsg in, out;
    in.level = ib::mw::logging::Level::Trace;
    ib::mw::logging::SourceLoc loc;
    loc.filename = "somefile.txt";
    loc.funcname ="TEST(LoggingSerdes)";
    loc.line = 15;
    in.source = loc;
    in.payload = "Hello, logger!";

    Serialize(buffer, in);
    Deserialize(buffer, out);
    ASSERT_EQ(in, out);
}
