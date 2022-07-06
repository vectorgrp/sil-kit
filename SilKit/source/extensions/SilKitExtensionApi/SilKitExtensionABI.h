// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#ifndef __SILKIT_EXTENSION_ABI_H__
#define __SILKIT_EXTENSION_ABI_H__

/*! SILKIT Extension Libraries
 *
 * This header defines the shared library interface in terms of C symbols, which
 * allow creation of SILKIT extensions as dynamic libraries.
 * You have to implement the C functions listed below in your own library.
 *
 * Version Compatibility:
 * - SILKIT version must match, otherwise the extension will not be loaded.
 * - newer versions of SilKitExtensionDescriptor must not change the existing struct
 *   layout, but may append to it.
 */

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>
#include <stddef.h> //NULL

#if defined(_WIN32)
#   define VIBE_API  __declspec(dllexport)
#   define VIBE_CABI __cdecl
#else
#   define VIBE_API  __attribute__((visibility ("default")))
#   define VIBE_CABI
#endif

//! \brief The SilKitExtensionDescriptor is a simple C structure that contains
//!        interoperability information.
//! This structure is provided by the dynamic library at load time, and the
//! extension mechanism verifies that the extension is compatible with the host
//! SILKIT system, before accessing the Create/Release routines.

struct SilKitExtensionDescriptor {
    uint32_t    silkit_version_major;
    uint32_t    silkit_version_minor;
    uint32_t    silkit_version_patch;
    const char* extension_name;
    const char* vendor_name;
    const char* system_name; // eg, distribution or operating system name
    uint32_t build_infos[5]; // See SilKitExtension.hpp for values
};
typedef struct SilKitExtensionDescriptor SilKitExtensionDescriptor_t;

//! \brief The silkit_extension_descriptor marks the entry point for an extension
//         shared library.
VIBE_API extern const SilKitExtensionDescriptor_t silkit_extension_descriptor;

//! \brief Handle to a C++ instance of an extension
#define VIBE_EXTENSION_HANDLE void*
#define VIBE_INVALID_HANDLE NULL

//! \brief Extension API entry point: Instantiates the extension implementation
//         given the descriptor.
VIBE_API VIBE_EXTENSION_HANDLE VIBE_CABI CreateExtension();

//! \brief Extension API entry point: Releases the extension implementation.
VIBE_API void VIBE_CABI ReleaseExtension(VIBE_EXTENSION_HANDLE extension);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __SILKIT_EXTENSION_ABI_H__
