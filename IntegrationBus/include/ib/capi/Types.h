// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "IbMacros.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

typedef struct ib_SimulationParticipant ib_SimulationParticipant;

typedef int32_t ib_ReturnCode;

#define ib_ReturnCode_SUCCESS            ((ib_ReturnCode) 0)
#define ib_ReturnCode_UNSPECIFIEDERROR   ((ib_ReturnCode) 1)
#define ib_ReturnCode_NOTSUPPORTED       ((ib_ReturnCode) 2)
#define ib_ReturnCode_NOTIMPLEMENTED     ((ib_ReturnCode) 3)
#define ib_ReturnCode_BADPARAMETER       ((ib_ReturnCode) 4)
#define ib_ReturnCode_BUFFERTOOSMALL     ((ib_ReturnCode) 5)
#define ib_ReturnCode_TIMEOUT            ((ib_ReturnCode) 6)
#define ib_ReturnCode_UNSUPPORTEDSERVICE ((ib_ReturnCode) 7)

typedef uint64_t ib_NanosecondsTime;

typedef struct
{
    uint8_t MajorVersion;
    uint8_t MinorVersion;
} ib_Version;

struct ib_ByteVector
{
    const uint8_t* pointer;
    size_t size;
};
typedef struct ib_ByteVector ib_ByteVector;

typedef uint8_t ib_Bool;
#define ib_True  ((ib_Bool)1)
#define ib_False ((ib_Bool)0)

IB_END_DECLS

#pragma pack(pop)
