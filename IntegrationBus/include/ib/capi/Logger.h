/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Utils.h"

#pragma pack(push)
#pragma pack(8)

__IB_BEGIN_DECLS

/*! \brief Information level of log messages
*/
typedef uint32_t ib_LoggingLevel;
#define ib_LoggingLevel_Trace ((uint32_t) 0) //!< Detailed debug-level messages
#define ib_LoggingLevel_Debug ((uint32_t) 1) //!< Normal debug-level messages
#define ib_LoggingLevel_Info ((uint32_t) 2) //!< Informational content
#define ib_LoggingLevel_Warn ((uint32_t) 3) //!< Warnings
#define ib_LoggingLevel_Error ((uint32_t) 4) //!< Non-critical errors
#define ib_LoggingLevel_Critical ((uint32_t) 5) //!< Critical errors
#define ib_LoggingLevel_Off ((uint32_t) 6) //!< Logging is disabled

typedef struct ib_Logger ib_Logger;

typedef ib_ReturnCode(*ib_Logger_Log_t)(ib_Logger* self, ib_LoggingLevel level, const char* message);
/*! \brief Log a message with a specified level
 *
 * \param level The log level for the message
 * \param message The message which shall be logged.
 */
CIntegrationBusAPI ib_ReturnCode ib_Logger_Log(ib_Logger* self, ib_LoggingLevel level, const char* message);

__IB_END_DECLS

#pragma pack(pop)
