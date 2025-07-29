// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

    ioContext.Dispatch([&flag] { flag = true; });
    ASSERT_FALSE(flag);

    ioContext.Run();
    ASSERT_TRUE(flag);
}


void CheckIoContextPostBeforeRunIsExecutedAtRun(VSilKit::IIoContext& ioContext)
{
    bool flag{false};

    ioContext.Post([&flag] { flag = true; });
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
