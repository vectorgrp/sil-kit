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

#pragma once

#ifdef __cplusplus
#    include <cstdint>
#else
#    include <stdint.h>
#endif

#ifdef __cplusplus
#    define SILKIT_BEGIN_DECLS \
        extern "C" \
        {
#    define SILKIT_END_DECLS }
#else
#    define SILKIT_BEGIN_DECLS
#    define SILKIT_END_DECLS
#endif

#ifdef SILKIT_BUILD_STATIC
#    define SilKitAPI
#elif defined(EXPORT_SilKitAPI)
// define compiler specific export / import attributes
// define SilKitAPI as EXPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllexport)
#    elif defined(__GNUC__)
#        define SilKitAPI __attribute__((visibility("default")))
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link export semantics.
#    endif
#else
// declare SilKitAPI as IMPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllimport)
#    elif defined(__GNUC__)
#        define SilKitAPI
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link import semantics.
#    endif
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
#    define BIT(X) (UINTMAX_C(1) << (X))
#endif

#define SILKIT_UNUSED_ARG(X) (void)(X)
