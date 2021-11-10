// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <stddef.h>

#pragma pack(push)
#pragma pack(8)

#ifdef __cplusplus
#define __IB_BEGIN_DECLS extern "C" {
#define __IB_END_DECLS }
#else
#define __IB_BEGIN_DECLS
#define __IB_END_DECLS
#endif

// define compiler specific export / import attributes
#if defined CIntegrationBusAPI_STATIC
#    define CIntegrationBusAPI
#else
#    ifdef CIntegrationBusAPI_EXPORT
//#        define CIntegrationBusAPI as EXPORT
#        if defined(_MSC_VER)
#            define CIntegrationBusAPI __declspec(dllexport)
#        elif defined(__GNUC__)
#            define CIntegrationBusAPI __attribute__((visibility("default")))
#        else
#            define CIntegrationBusAPI
#            pragma warning Unknown dynamic link export semantics.
#        endif
#    else
//#        declare CIntegrationBusAPI as IMPORT
#        if defined(_MSC_VER)
#            define CIntegrationBusAPI __declspec(dllimport)
#        elif defined(__GNUC__)
#            define CIntegrationBusAPI
#        else
#            define CIntegrationBusAPI
#            pragma warning Unknown dynamic link import semantics.
#        endif
#    endif  
#endif  

__IB_BEGIN_DECLS

typedef struct ib_SimulationParticipant ib_SimulationParticipant;

typedef int32_t ib_ReturnCode;

#define ib_ReturnCode_SUCCESS            ((int32_t) 0)
#define ib_ReturnCode_UNSPECIFIEDERROR   ((int32_t) 1)
#define ib_ReturnCode_NOTSUPPORTED       ((int32_t) 2)
#define ib_ReturnCode_NOTIMPLEMENTED     ((int32_t) 3)
#define ib_ReturnCode_BADPARAMETER       ((int32_t) 4)
#define ib_ReturnCode_BUFFERTOOSMALL     ((int32_t) 5)
#define ib_ReturnCode_TIMEOUT            ((int32_t) 6)
#define ib_ReturnCode_UNSUPPORTEDSERVICE ((int32_t) 7)

typedef uint64_t ib_NanosecondsTime;

typedef struct
{
    uint8_t MajorVersion;
    uint8_t MinorVersion;
} ib_Version;

struct ib_ByteVector
{
    uint8_t * pointer;
    size_t size;
};
typedef struct ib_ByteVector ib_ByteVector;

__IB_END_DECLS

#pragma pack(pop)
