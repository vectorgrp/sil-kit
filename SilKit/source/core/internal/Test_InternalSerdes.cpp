// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "InternalSerdes.hpp"

TEST(Test_InternalSerdes, Mw_EndpointAddress)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::EndpointAddress in{3, 7};
    SilKit::Core::EndpointAddress out{1, 1};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(Test_InternalSerdes, Mw_EndpointAddress_multiple)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::EndpointAddress in1{3, 7};
    SilKit::Core::EndpointAddress in2{4, 8};
    SilKit::Core::EndpointAddress in3{5, 9};

    SilKit::Core::EndpointAddress out1{1, 1};
    SilKit::Core::EndpointAddress out2{2, 2};
    SilKit::Core::EndpointAddress out3{3, 3};

    buffer << in1 << in2 << in3;
    buffer >> out1 >> out2 >> out3;

    EXPECT_EQ(in1, out1);
    EXPECT_EQ(in2, out2);
    EXPECT_EQ(in3, out3);
}
