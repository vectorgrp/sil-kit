// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
#define SILKIT_BEGIN_DECLS \
    extern "C" \
    {
#define SILKIT_END_DECLS }
#else
#define SILKIT_BEGIN_DECLS
#define SILKIT_END_DECLS
#endif

#ifdef SILKIT_BUILD_STATIC
#define SilKitAPI
#elif defined(EXPORT_SilKitAPI)
// define compiler specific export / import attributes
// define SilKitAPI as EXPORT
#if defined(_WIN32)
#define SilKitAPI __declspec(dllexport)
#elif defined(__GNUC__)
#define SilKitAPI __attribute__((visibility("default")))
#else
#define SilKitAPI
#pragma warning Unknown dynamic link export semantics.
#endif
#else
// declare SilKitAPI as IMPORT
#if defined(_WIN32)
#define SilKitAPI __declspec(dllimport)
#elif defined(__GNUC__)
#define SilKitAPI
#else
#define SilKitAPI
#pragma warning Unknown dynamic link import semantics.
#endif
#endif

#ifdef _WIN32
#define SilKitCALL __cdecl
#define SilKitFPTR __cdecl
#else
#define SilKitCALL
#define SilKitFPTR
#endif

// Utilities for more readable definitions
#ifndef BIT
#define BIT(X) (UINTMAX_C(1) << (X))
#endif

#define SILKIT_UNUSED_ARG(X) (void)(X)
