// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gtest/gtest.h"

#include "InternalSerdes.hpp"

TEST(MwVAsioSerdes, Mw_EndpointAddress)
{
    ib::mw::MessageBuffer buffer;

    ib::mw::EndpointAddress in{3,7};
    ib::mw::EndpointAddress out{1,1};
    
    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, Mw_EndpointAddress_multiple)
{
    ib::mw::MessageBuffer buffer;

    ib::mw::EndpointAddress in1{3,7};
    ib::mw::EndpointAddress in2{4,8};
    ib::mw::EndpointAddress in3{5,9};

    ib::mw::EndpointAddress out1{1,1};
    ib::mw::EndpointAddress out2{2,2};
    ib::mw::EndpointAddress out3{3,3};

    buffer << in1 << in2 << in3;
    buffer >> out1 >> out2 >> out3;

    EXPECT_EQ(in1, out1);
    EXPECT_EQ(in2, out2);
    EXPECT_EQ(in3, out3);
}

