// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// This translation unit compiles the rapidyaml (single-header) implementation.
//
// rapidyaml is a single-header library: defining RYML_SINGLE_HDR_DEFINE_NOW
// before including the header pulls in its implementation. Add THIS file to
// exactly ONE target in your build so that the rapidyaml symbols
// (ryml::parse_in_arena, the emitters, the tree internals, ...) are defined
// exactly once. The SilKit::Yaml INTERFACE target supplies the include path and
// the required RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1 definition; make sure the
// target that compiles this file links SilKit::Yaml so it inherits both.
//
// If your project already compiles the rapidyaml implementation elsewhere, do
// NOT add this file (you would get duplicate-symbol linker errors).

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml.hpp"
