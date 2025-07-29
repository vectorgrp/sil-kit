// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>

#pragma once
namespace SilKit {
namespace Util {

// Set the name for the current thread.
void SetThreadName(const std::string& threadName);

} // namespace Util
} // namespace SilKit