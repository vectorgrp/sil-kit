// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT


#include <string>
#include <fstream>


namespace SilKit {
namespace Util {

auto OpenIFStream(const std::string& path) -> std::ifstream;
auto ReadTextFile(const std::string& path) -> std::string;

} // namespace Util
} // namespace SilKit
