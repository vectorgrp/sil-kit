// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

#define SilKit_ReturnCode_SUCCESS ((SilKit_ReturnCode)0)
#define SilKit_ReturnCode_UNSPECIFIEDERROR ((SilKit_ReturnCode)1)
#define SilKit_ReturnCode_NOTSUPPORTED ((SilKit_ReturnCode)2)
#define SilKit_ReturnCode_NOTIMPLEMENTED ((SilKit_ReturnCode)3)
#define SilKit_ReturnCode_BADPARAMETER ((SilKit_ReturnCode)4)
#define SilKit_ReturnCode_BUFFERTOOSMALL ((SilKit_ReturnCode)5)
#define SilKit_ReturnCode_TIMEOUT ((SilKit_ReturnCode)6)
#define SilKit_ReturnCode_UNSUPPORTEDSERVICE ((SilKit_ReturnCode)7)

// The following return codes have an corresponding specific SIL Kit exception.
// If an error occurs and a specific exception is thrown, it is caught in
// CapiImpl.hpp and the code defined here is returned.
// This completes the error handling for the usage of the C-API.

// For the C++-Api, the return code is translated back to the specific SIL Kit
// exception and thrown in ThrowOnError.hpp.

#define SilKit_ReturnCode_WRONGSTATE ((SilKit_ReturnCode)8)          // SilKit::StateError
#define SilKit_ReturnCode_TYPECONVERSIONERROR ((SilKit_ReturnCode)9) // SilKit::TypeConversionError
#define SilKit_ReturnCode_CONFIGURATIONERROR ((SilKit_ReturnCode)10) // SilKit::ConfigurationError
#define SilKit_ReturnCode_PROTOCOLERROR ((SilKit_ReturnCode)11)      // SilKit::ProtocolError
#define SilKit_ReturnCode_ASSERTIONERROR ((SilKit_ReturnCode)12)     // SilKit::AssertionError
#define SilKit_ReturnCode_EXTENSIONERROR ((SilKit_ReturnCode)13)     // SilKit::ExtensionError
#define SilKit_ReturnCode_LOGICERROR ((SilKit_ReturnCode)14)         // SilKit::LogicError
#define SilKit_ReturnCode_LENGTHERROR ((SilKit_ReturnCode)15)        // SilKit::LengthError
#define SilKit_ReturnCode_OUTOFRANGEERROR ((SilKit_ReturnCode)16)    // SilKit::OutOfRangeError

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
#define SilKit_LabelKind_Undefined ((uint32_t)0) //!< Undefined
#define SilKit_LabelKind_Optional ((uint32_t)1)  //!< If this label is available, its value must match
#define SilKit_LabelKind_Mandatory ((uint32_t)2) //!< This label must be available and its value must match

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
#define SilKit_True ((SilKit_Bool)1)
#define SilKit_False ((SilKit_Bool)0)

// NB: Should map to SilKit::Services::TransmitDirection
/*! \brief An enum type defining the transmit direction within the simulation */
typedef uint8_t SilKit_Direction;
#define SilKit_Direction_Undefined ((SilKit_Direction)0)
#define SilKit_Direction_Send ((SilKit_Direction)1)
#define SilKit_Direction_Receive ((SilKit_Direction)2)
#define SilKit_Direction_SendReceive ((SilKit_Direction)3)

typedef uint64_t SilKit_HandlerId;

SILKIT_END_DECLS

#pragma pack(pop)
