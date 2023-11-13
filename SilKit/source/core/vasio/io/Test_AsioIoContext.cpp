// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "MakeAsioIoContext.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


auto MakeIoContextWithDefaultSocketOptions() -> std::unique_ptr<VSilKit::IIoContext>
{
    VSilKit::AsioSocketOptions asioSocketOptions{};
    return VSilKit::MakeAsioIoContext(asioSocketOptions);
}


void CheckIoContextDispatchBeforeRunIsExecutedAtRun(VSilKit::IIoContext& ioContext)
{
    bool flag{false};

    ioContext.Dispatch([&flag] {
        flag = true;
    });
    ASSERT_FALSE(flag);

    ioContext.Run();
    ASSERT_TRUE(flag);
}


void CheckIoContextPostBeforeRunIsExecutedAtRun(VSilKit::IIoContext& ioContext)
{
    bool flag{false};

    ioContext.Post([&flag] {
        flag = true;
    });
    ASSERT_FALSE(flag);

    ioContext.Run();
    ASSERT_TRUE(flag);
}


TEST(Test_AsioIoContext, make_io_context_with_default_socket_options)
{
    auto ioContext{MakeIoContextWithDefaultSocketOptions()};
    ASSERT_NE(ioContext, nullptr);

    // sanity checks
    CheckIoContextDispatchBeforeRunIsExecutedAtRun(*ioContext);
    CheckIoContextPostBeforeRunIsExecutedAtRun(*ioContext);
}


} // namespace
