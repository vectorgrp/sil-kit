// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "IbMacros.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

typedef struct ib_Participant ib_Participant;

typedef int32_t ib_ReturnCode;

#define ib_ReturnCode_SUCCESS            ((ib_ReturnCode) 0)
#define ib_ReturnCode_UNSPECIFIEDERROR   ((ib_ReturnCode) 1)
#define ib_ReturnCode_NOTSUPPORTED       ((ib_ReturnCode) 2)
#define ib_ReturnCode_NOTIMPLEMENTED     ((ib_ReturnCode) 3)
#define ib_ReturnCode_BADPARAMETER       ((ib_ReturnCode) 4)
#define ib_ReturnCode_BUFFERTOOSMALL     ((ib_ReturnCode) 5)
#define ib_ReturnCode_TIMEOUT            ((ib_ReturnCode) 6)
#define ib_ReturnCode_UNSUPPORTEDSERVICE ((ib_ReturnCode) 7)
#define ib_ReturnCode_WRONGSTATE         ((ib_ReturnCode) 8) // Returned on exception ib::StateError (CapiImpl.h)

typedef uint64_t ib_NanosecondsTime;

typedef struct
{
    uint8_t MajorVersion;
    uint8_t MinorVersion;
} ib_Version;

struct ib_ByteVector
{
    const uint8_t* data;
    size_t size;
};
typedef struct ib_ByteVector ib_ByteVector;


/*! \brief Key-value pair */
typedef struct ib_KeyValuePair
{
    const char* key;
    const char* value;
} ib_KeyValuePair;

/*! \brief Key-value list */
typedef struct ib_KeyValueList
{
    size_t numLabels;
    ib_KeyValuePair labels[1];
} ib_KeyValueList;

/*! \brief String list */
typedef struct ib_StringList
{
    size_t numStrings;
    const char* strings[1];
} ib_StringList;

typedef uint8_t ib_Bool;
#define ib_True  ((ib_Bool)1)
#define ib_False ((ib_Bool)0)

// NB: Should map to ib::sim::TransmitDirection
/*! \brief An enum type defining the transmit direction within the simulation */
typedef uint8_t ib_Direction;
#define ib_Direction_Undefined      ((ib_Direction) 0)
#define ib_Direction_Send           ((ib_Direction) 1)
#define ib_Direction_Receive        ((ib_Direction) 2)
#define ib_Direction_SendReceive    ((ib_Direction) 3)

IB_END_DECLS

#pragma pack(pop)
