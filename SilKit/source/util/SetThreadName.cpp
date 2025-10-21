// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SetThreadName.hpp"
#include "Assert.hpp"

#include "silkit/capi/SilKitMacros.h"

#if _WIN32
#include <windows.h> // HANDLE, PCWSTR
#else                // posix
#include <pthread.h>
#endif

namespace SilKit {
namespace Util {

#if defined(_WIN32)

#if defined(__MINGW32__)
//squelch mingw crossbuild warnings
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

typedef HRESULT(WINAPI* SetThreadDescriptionProc)(HANDLE hThread, PCWSTR lpThreadDescription);

inline std::wstring ToWString(const char* str)
{
    std::wstring result;
    if (str != nullptr)
    {
        int len = ::MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
        if (len > 0)
        {
            result.resize(len); // len includes the terminating null character
            ::MultiByteToWideChar(CP_ACP, 0, str, -1, &(result[0]), len);
            result.pop_back(); // remove the terminating null character
        }
    }
    return result;
}

static SetThreadDescriptionProc GetFunctionPointer_SetThreadDescription()
{
    // NB: The Windows function SetThreadDescription is new in 'Windows 10 version 1607' and not available on older systems
    HMODULE kernel32ModuleHandle = GetModuleHandleW(L"kernel32.dll");
    return (SetThreadDescriptionProc)::GetProcAddress(kernel32ModuleHandle, "SetThreadDescription");
}

void SetThreadName(const std::string& threadName)
{
    static SetThreadDescriptionProc sSetThreadDescriptionProc = GetFunctionPointer_SetThreadDescription();

    if (sSetThreadDescriptionProc != nullptr)
    {
        HANDLE threadHandle = GetCurrentThread();
        std::wstring threadnameW = ToWString(threadName.c_str());
        HRESULT hr = (*sSetThreadDescriptionProc)(threadHandle, threadnameW.c_str());
        (void)hr;
        SILKIT_ASSERT(SUCCEEDED(hr));
    }
}

#else

void SetThreadName(const std::string& threadName)
{
    int rc{0};
    // NB: On Linux the length of a thread name is restricted to 16 characters including the terminating null byte.
    SILKIT_ASSERT(threadName.size() < 16);

    pthread_t thisThread = pthread_self();

#if defined(__linux__) || defined(__QNX__) || defined(__FreeBSD__)
    rc = pthread_setname_np(thisThread, threadName.c_str());
#elif defined(__NetBSD__)
    rc = pthread_setname_np(thisThread, threadName.c_str(), nullptr);
#elif defined(__APPLE__)
    SILKIT_UNUSED_ARG(thisThread);
    rc = pthread_setname_np(threadName.c_str());
#endif
    // The function pthread_setname_np fails if the length of the specified name exceeds the allowed
    // limit. (16 characters including the terminating null byte)
    SILKIT_ASSERT(rc == 0);
    (void)rc;
}

#endif

} // namespace Util
} // namespace SilKit
