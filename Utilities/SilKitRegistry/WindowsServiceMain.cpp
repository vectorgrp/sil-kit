// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/SilKitMacros.hpp"
#include "WindowsServiceMain.hpp"

#if defined(HAS_SILKIT_REGISTRY_WINDOWS_SERVICE)

#    include "silkit/participant/exception.hpp"

#    include <iostream>
#    include <string>
#    include <future>
#    include <memory>

#    include <windows.h>
#    include <accctrl.h>
#    include <aclapi.h>
#    include <sddl.h>

// USAGE AS WINDOWS SERVICE
//
// Create the Windows Service using sc.exe (the space after the '=' is MANDATORY!):
//
//     sc.exe create VectorSilKitRegistry
//             binPath= "D:\VFS\REGINS\sil-kit\_build\d-vs2022-64\Debug\sil-kit-registry.exe --windows-service --listen-uri silkit://0.0.0.0:8500"
//             DisplayName= "Vector SIL Kit Registry"
//
// Start the service using the Task Manager, Service panel, or using sc.exe:
//
//     sc.exe start VectorSilKitRegistry
//
// Delete the service using sc.exe:
//
//     sc.exe delete VectorSilKitRegistry

namespace {

// ============================================================
//  Global Variables
// ============================================================

SilKitRegistry::StartFunction gStartFunction;
SERVICE_STATUS_HANDLE gServiceStatusHandle{nullptr};
SERVICE_STATUS gServiceStatus{};
std::promise<void> gServiceStopped;

// ============================================================
//  Internal Functions
// ============================================================

auto GetServiceName() -> LPSTR
{
    return const_cast<LPSTR>(TEXT("VectorSilKitRegistry"));
}

VOID SvcReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gServiceStatus.dwCurrentState = dwCurrentState;
    gServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    gServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        gServiceStatus.dwControlsAccepted = 0;
    }
    else
    {
        gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
    {
        gServiceStatus.dwCheckPoint = 0;
    }
    else
    {
        gServiceStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the SCM.
    SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
}

VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv)
{
    SILKIT_UNUSED_ARG(dwArgc);
    SILKIT_UNUSED_ARG(lpszArgv);

    SilKitRegistry::RegistryInstance registry;

    if (gStartFunction == nullptr)
    {
        SvcReportStatus(SERVICE_STOPPED, ERROR_INTERNAL_ERROR, 0);
        return;
    }

    try
    {
        registry = gStartFunction();
    }
    catch (...)
    {
        SvcReportStatus(SERVICE_STOPPED, ERROR_INTERNAL_ERROR, 0);
        return;
    }

    if (registry._registry == nullptr)
    {
        SvcReportStatus(SERVICE_STOPPED, ERROR_INTERNAL_ERROR, 0);
        return;
    }

    SvcReportStatus(SERVICE_RUNNING, NO_ERROR, 0);

    try
    {
        gServiceStopped.get_future().get();
    }
    catch (...)
    {
        SvcReportStatus(SERVICE_STOPPED, ERROR_INTERNAL_ERROR, 0);
        return;
    }

    registry = {};

    // report that the service has stopped
    SvcReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

VOID WINAPI SvcControlHandler(DWORD controlCode)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_STOP:
        SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        try
        {
            gServiceStopped.set_value();
        }
        catch (...)
        {
            // ignore any exception from setting the promise
        }
        break;

    default: break;
    }
}

// ============================================================
//  Main Service Entrypoint
// ============================================================

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    gServiceStatusHandle = RegisterServiceCtrlHandler(GetServiceName(), &SvcControlHandler);
    if (!gServiceStatusHandle)
    {
        std::cerr << "Error: RegisterServiceCtrlHandler failed. Please start the SIL Kit Registry from the service "
                     "manager when using the --windows-service flag."
                  << std::endl;
        return;
    }

    gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gServiceStatus.dwServiceSpecificExitCode = 0;

    SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    SvcInit(dwArgc, lpszArgv);
}

// ============================================================
//  Win32 Error Handling Helpers
// ============================================================

struct SilKitWinError : SilKit::SilKitError
{
    SilKitWinError(std::string functionName)
        : SilKitWinError(std::move(functionName), GetLastError())
    {
    }

    SilKitWinError(std::string functionName, DWORD error)
        : SilKit::SilKitError(functionName + ": " + std::to_string(error))
    {
    }
};

// ============================================================
//  Win32 Memory Management Helpers
// ============================================================

template <typename T>
using WinLocalPtr = std::unique_ptr<T, std::function<void(T *)>>;

template <typename T>
auto MakeWinLocalPtr(T *pointer) -> WinLocalPtr<T>
{
    return WinLocalPtr<T>(pointer, [](T *pointer) {
        LocalFree(pointer);
    });
}

// ============================================================
//  Win32 Process Access Rights Helpers
// ============================================================

auto GetCurrentProcessDacl() -> PACL
{
    PACL dacl;

    const auto result = GetSecurityInfo(
        GetCurrentProcess(), // process handle (result of GetCurrentProcess always has PROCESS_ALL_ACCESS rights)
        SE_KERNEL_OBJECT, // object type
        DACL_SECURITY_INFORMATION, // security info
        nullptr, // owner
        nullptr, // group
        &dacl,
        nullptr, // sacl
        nullptr // security descriptor
    );

    if (result != ERROR_SUCCESS)
    {
        throw SilKitWinError("GetSecurityInfo", result);
    }

    return dacl;
}

auto AddProcessQueryLimitedInformationToDacl(PACL dacl) -> WinLocalPtr<ACL>
{
    const auto pSid = []() -> WinLocalPtr<VOID> {
        constexpr auto everyoneSidText = TEXT("S-1-1-0");

        PSID pSid;
        if (ConvertStringSidToSid(everyoneSidText, &pSid) == 0)
        {
            throw SilKitWinError("ConvertStringSidToSid");
        }

        return MakeWinLocalPtr(pSid);
    }();

    // the last member is used "as a union" (but is not a union, therefore the cast is required)
    // see https://learn.microsoft.com/en-us/windows/win32/api/accctrl/ns-accctrl-trustee_a
    const TRUSTEE trustee{nullptr, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_SID, TRUSTEE_IS_GROUP,
                          reinterpret_cast<LPSTR>(pSid.get())};

    EXPLICIT_ACCESS explicitAccess{PROCESS_QUERY_LIMITED_INFORMATION, GRANT_ACCESS, NO_INHERITANCE, trustee};

    PACL newAcl;

    const auto result = SetEntriesInAcl(1, &explicitAccess, dacl, &newAcl);

    if (result != ERROR_SUCCESS)
    {
        throw SilKitWinError("SetEntriesInAcl", result);
    }

    return MakeWinLocalPtr(newAcl);
}

void SetCurrentProcessDacl(PACL dacl)
{
    const auto result = SetSecurityInfo(
        GetCurrentProcess(), // process handle (result of GetCurrentProcess always has PROCESS_ALL_ACCESS rights)
        SE_KERNEL_OBJECT, // object type
        DACL_SECURITY_INFORMATION, // security info
        nullptr, // owner
        nullptr, // group
        dacl,
        nullptr // sacl
    );

    if (result != ERROR_SUCCESS)
    {
        throw SilKitWinError("SetSecurityInfo", result);
    }
}

void AllowProcessQueryLimitedInformation()
{
    const auto originalDacl = GetCurrentProcessDacl();

    auto modifiedDacl = AddProcessQueryLimitedInformationToDacl(originalDacl);

    SetCurrentProcessDacl(modifiedDacl.get());
}

} // namespace

namespace SilKitRegistry {

void RunWindowsService(StartFunction startFunction)
{
    AllowProcessQueryLimitedInformation();

    gStartFunction = std::move(startFunction);

    SERVICE_TABLE_ENTRY serviceTable[] = {
        {GetServiceName(), &SvcMain},
        {nullptr, nullptr},
    };

    // The call to StartServiceCtrlDispatcher returns only after the service was stopped.
    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        std::cerr << "Error: StartServiceCtrlDispatcher failed. Please start the SIL Kit Registry from the service "
                     "manager when using the --windows-service flag."
                  << std::endl;
    }
}

} // namespace SilKitRegistry

#endif
