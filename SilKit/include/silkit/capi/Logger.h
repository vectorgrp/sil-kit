// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

/*! \brief Information level of log messages
*/
typedef uint32_t SilKit_LoggingLevel;
#define SilKit_LoggingLevel_Trace ((uint32_t)0)        //!< Detailed debug-level messages
#define SilKit_LoggingLevel_Debug ((uint32_t)1)        //!< Normal debug-level messages
#define SilKit_LoggingLevel_Info ((uint32_t)2)         //!< Informational content
#define SilKit_LoggingLevel_Warn ((uint32_t)3)         //!< Warnings
#define SilKit_LoggingLevel_Error ((uint32_t)4)        //!< Non-critical errors
#define SilKit_LoggingLevel_Critical ((uint32_t)5)     //!< Critical errors
#define SilKit_LoggingLevel_Off ((uint32_t)0xffffffff) //!< Logging is disabled

typedef struct SilKit_Logger SilKit_Logger;

/*! \brief Log a message with a specified level
 *
 * \param logger The logger to use.
 * \param level The log level for the message.
 * \param message The message which shall be logged (UTF-8).
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Logger_Log(SilKit_Logger* logger, SilKit_LoggingLevel level,
                                                         const char* message);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Logger_Log_t)(SilKit_Logger* logger, SilKit_LoggingLevel level,
                                                           const char* message);

/*! \brief Get the lowest configured log level of the log sinks
 *
 * \param logger The logger for which the log level should be obtained.
 * \param outLevel A pointer to a logging level where the result will be stored.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Logger_GetLogLevel(SilKit_Logger* logger, SilKit_LoggingLevel* outLevel);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Logger_GetLogLevel_t)(SilKit_Logger* logger,
                                                                   SilKit_LoggingLevel* outLevel);

SILKIT_END_DECLS

#pragma pack(pop)
