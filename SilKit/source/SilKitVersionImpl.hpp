// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <cstdint>

namespace SilKit {
namespace Version {

auto MajorImpl() -> uint32_t;

auto MinorImpl() -> uint32_t;

auto PatchImpl() -> uint32_t;

auto BuildNumberImpl() -> uint32_t;

auto StringImpl() -> const char*;

auto VersionSuffixImpl() -> const char*;

auto GitHashImpl() -> const char*;

} // namespace Version
} // namespace SilKit
