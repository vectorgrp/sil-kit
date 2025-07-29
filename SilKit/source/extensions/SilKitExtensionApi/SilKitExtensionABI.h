// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#ifndef __SILKIT_EXTENSION_ABI_H__
#define __SILKIT_EXTENSION_ABI_H__

/*! SIL Kit Extension Libraries
 *
 * This header defines the shared library interface in terms of C symbols, which
 * allow creation of SIL Kit extensions as dynamic libraries.
 * You have to implement the C functions listed below in your own library.
 *
 * Version Compatibility:
 * - SIL Kit version must match, otherwise the extension will not be loaded.
 * - newer versions of SilKitExtensionDescriptor must not change the existing struct
 *   layout, but may append to it.
 */

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <stdint.h>
#include <stddef.h> //NULL

#if defined(_WIN32)
#define SILEXT_API __declspec(dllexport)
#define SILEXT_CABI __cdecl
#else
#define SILEXT_API __attribute__((visibility("default")))
#define SILEXT_CABI
#endif

    //! \brief The SilKitExtensionDescriptor is a simple C structure that contains
    //!        interoperability information.
    //! This structure is provided by the dynamic library at load time, and the
    //! extension mechanism verifies that the extension is compatible with the host
    //! SIL Kit system, before accessing the Create/Release routines.

    struct SilKitExtensionDescriptor
    {
        uint32_t silkit_version_major;
        uint32_t silkit_version_minor;
        uint32_t silkit_version_patch;
        const char* extension_name;
        const char* vendor_name;
        const char* system_name; // eg, distribution or operating system name
        uint32_t build_infos[5]; // See SilKitExtension.hpp for values
    };
    typedef struct SilKitExtensionDescriptor SilKitExtensionDescriptor_t;

    //! \brief The silkit_extension_descriptor marks the entry point for an extension
    //         shared library.
    SILEXT_API extern const SilKitExtensionDescriptor_t silkit_extension_descriptor;

//! \brief Handle to a C++ instance of an extension
#define SILEXT_EXTENSION_HANDLE void*
#define SILEXT_INVALID_HANDLE NULL

    //! \brief Extension API entry point: Instantiates the extension implementation
    //         given the descriptor.
    SILEXT_API SILEXT_EXTENSION_HANDLE SILEXT_CABI CreateExtension();

    //! \brief Extension API entry point: Releases the extension implementation.
    SILEXT_API void SILEXT_CABI ReleaseExtension(SILEXT_EXTENSION_HANDLE extension);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __SILKIT_EXTENSION_ABI_H__
