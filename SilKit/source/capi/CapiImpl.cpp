// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "util/StringHelpers.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "fmt/format.h"
#include "fmt/ostream.h"

namespace VSilKit {

void ApiTraceEventImpl(const std::string_view func, const std::string_view data)
{
    thread_local std::string message;

    message.clear();
    message.append(R"({"thread":)");
    fmt::format_to(std::back_inserter(message), "{}", fmt::streamed(std::this_thread::get_id()));
    message.append(R"(,"func":")");
    SilKit::Util::AppendEscapedJsonStringTo(func, message);
    message.append(R"(","data":")");
    SilKit::Util::AppendEscapedJsonStringTo(data, message);
    message.append(R"("})");

    std::cerr << message << '\n';
}

} // namespace VSilKit
