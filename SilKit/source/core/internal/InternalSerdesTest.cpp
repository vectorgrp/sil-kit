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

#include "gtest/gtest.h"

#include "InternalSerdes.hpp"

TEST(MwVAsioSerdes, Mw_EndpointAddress)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::EndpointAddress in{3,7};
    SilKit::Core::EndpointAddress out{1,1};
    
    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, Mw_EndpointAddress_multiple)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::EndpointAddress in1{3,7};
    SilKit::Core::EndpointAddress in2{4,8};
    SilKit::Core::EndpointAddress in3{5,9};

    SilKit::Core::EndpointAddress out1{1,1};
    SilKit::Core::EndpointAddress out2{2,2};
    SilKit::Core::EndpointAddress out3{3,3};

    buffer << in1 << in2 << in3;
    buffer >> out1 >> out2 >> out3;

    EXPECT_EQ(in1, out1);
    EXPECT_EQ(in2, out2);
    EXPECT_EQ(in3, out3);
}

