// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SetThreadName.hpp"
#include "Assert.hpp"

#if _WIN32
#include <windows.h> // HANDLE, PCWSTR
#else
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
        HANDLE       threadHandle = GetCurrentThread();
        std::wstring threadnameW = ToWString(threadName.c_str());
        HRESULT      hr = (*sSetThreadDescriptionProc)(threadHandle, threadnameW.c_str());
        (void)hr;
        SILKIT_ASSERT(SUCCEEDED(hr));
    }
}

#else

void SetThreadName(const std::string& threadName)
{
    // NB: On Linux the length of a thread name is restricted to 16 characters including the terminating null byte.
    SILKIT_ASSERT(threadName.size() < 16);

    pthread_t thisThread = pthread_self();

    int rc = pthread_setname_np(thisThread, threadName.c_str());
    // The function pthread_setname_np fails if the length of the specified name exceeds the allowed
    // limit. (16 characters including the terminating null byte)
    SILKIT_ASSERT(rc == 0); 
    (void)rc;
}

#endif

} // namespace Util
} // namespace SilKit
