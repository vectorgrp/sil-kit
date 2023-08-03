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

#include "Timer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace std::chrono_literals;

TEST(Test_Timer, ensure_util_timer_works)
{
    {
        //Make sure DTor is able to stop a running timer
        SilKit::Util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        std::promise<void> done;
        auto isDone = done.get_future();
        SilKit::Util::Timer timer;
        auto numCalls = 0u;
        auto cb = [&](const auto) {
          numCalls++;
          if (numCalls == 5)
          {
              timer.Stop();
              done.set_value();
          }
        };
        timer.WithPeriod(std::chrono::milliseconds(50), cb);
        ASSERT_TRUE(timer.IsActive());
        isDone.wait_for(1s);
        ASSERT_TRUE(!timer.IsActive());
        ASSERT_EQ((int)numCalls, 5);
    }
}

}
