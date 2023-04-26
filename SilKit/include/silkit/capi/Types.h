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
#include <stdint.h>
#include <stddef.h>
#include "SilKitMacros.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

typedef struct SilKit_ParticipantConfiguration SilKit_ParticipantConfiguration;

typedef struct SilKit_Participant SilKit_Participant;

typedef struct SilKit_Vendor_Vector_SilKitRegistry SilKit_Vendor_Vector_SilKitRegistry;

/*! \brief Opaque type. Used in functions prefixed with SilKit_Experimental_SystemController_....
 *
 * \warning This type is not part of the stable API of the SIL Kit. It may be removed at any time without prior notice.
 */
typedef struct SilKit_Experimental_SystemController SilKit_Experimental_SystemController;

typedef int32_t SilKit_ReturnCode;

#define SilKit_ReturnCode_SUCCESS            ((SilKit_ReturnCode) 0)
#define SilKit_ReturnCode_UNSPECIFIEDERROR   ((SilKit_ReturnCode) 1)
#define SilKit_ReturnCode_NOTSUPPORTED       ((SilKit_ReturnCode) 2)
#define SilKit_ReturnCode_NOTIMPLEMENTED     ((SilKit_ReturnCode) 3)
#define SilKit_ReturnCode_BADPARAMETER       ((SilKit_ReturnCode) 4)
#define SilKit_ReturnCode_BUFFERTOOSMALL     ((SilKit_ReturnCode) 5)
#define SilKit_ReturnCode_TIMEOUT            ((SilKit_ReturnCode) 6)
#define SilKit_ReturnCode_UNSUPPORTEDSERVICE ((SilKit_ReturnCode) 7)
#define SilKit_ReturnCode_WRONGSTATE         ((SilKit_ReturnCode) 8) // Returned on exception SilKit::StateError (CapiImpl.h)

typedef uint64_t SilKit_NanosecondsTime;

typedef struct
{
    uint8_t MajorVersion;
    uint8_t MinorVersion;
} SilKit_Version;

struct SilKit_ByteVector
{
    const uint8_t* data;
    size_t size;
};
typedef struct SilKit_ByteVector SilKit_ByteVector;


/*! \brief Information level of log messages
*/
typedef uint32_t SilKit_LabelKind;
#define SilKit_LabelKind_Undefined ((uint32_t) 0) //!< Undefined
#define SilKit_LabelKind_Optional ((uint32_t) 1) //!< If this label is available, its value must match
#define SilKit_LabelKind_Mandatory ((uint32_t) 2) //!< This label must be available and its value must match

/*! \brief Struct that contains a label as used in PubSub and RPC for matching publisher, subscribers, servers, and clients */
typedef struct SilKit_Label
{
    const char* key;
    const char* value;
    SilKit_LabelKind kind;
} SilKit_Label;

/*! \brief A list of data labels */
typedef struct SilKit_LabelList
{
    size_t numLabels;
    SilKit_Label* labels;
} SilKit_LabelList;

/*! \brief String list */
typedef struct SilKit_StringList
{
    size_t numStrings;
    char** strings;
} SilKit_StringList;

typedef uint8_t SilKit_Bool;
#define SilKit_True  ((SilKit_Bool)1)
#define SilKit_False ((SilKit_Bool)0)

// NB: Should map to SilKit::Services::TransmitDirection
/*! \brief An enum type defining the transmit direction within the simulation */
typedef uint8_t SilKit_Direction;
#define SilKit_Direction_Undefined      ((SilKit_Direction) 0)
#define SilKit_Direction_Send           ((SilKit_Direction) 1)
#define SilKit_Direction_Receive        ((SilKit_Direction) 2)
#define SilKit_Direction_SendReceive    ((SilKit_Direction) 3)

typedef uint64_t SilKit_HandlerId;

SILKIT_END_DECLS

#pragma pack(pop)
