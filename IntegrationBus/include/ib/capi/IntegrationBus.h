#pragma once
#include <stdint.h>
#include <limits.h>
#include "ib/capi/Utils.h"
#include "ib/capi/Can.h"
#include "ib/capi/Ethernet.h"
#include "ib/capi/InterfaceIdentifiers.h"


#ifdef __cplusplus
extern "C"
{
#endif

    typedef  ib_ReturnCode (*ib_ReturnCodeToString_t)(const char** outString, ib_ReturnCode returnCode);
    /*! \brief Get the corresponding static error string for a given return code.
    *
    * \param outString The pointer through which the resulting human readable error string will be returned.
    * \param returnCode The return code for which the string should be obtained.
    *
    * \ref ib_GetLastErrorString()
    *
    */
    CIntegrationBusAPI ib_ReturnCode ib_ReturnCodeToString(const char** outString, ib_ReturnCode returnCode);

    typedef  const char*(*ib_GetLastErrorString_t)();
    /*! \brief Get a human readable error description of the last error on the current thread.
    *
    * This method is intended to get specific error messages in case of a non success return code.
    * In comparision to ib_ReturnCodeToString this function returns dynamic and more specific error messages.
    *
    * \return A specific string containing the last error message of the current thrad.
    *
    */
    CIntegrationBusAPI const char* ib_GetLastErrorString();

    typedef void ib_SimulationParticipant;


    typedef ib_ReturnCode (*ib_SimulationParticipant_create_t)(ib_SimulationParticipant** outParticipant, const char* cJsonConfig, const char* cParticipantName, const char* cDomainId);
    /*! \brief Join the IB simulation with the domainId as a participant.
    *
    * Join the IB simulation and become a participant
    * based on the given configuration options.
    *
    * \param outParticipant The pointer through which the resulting simulation participant will be returned (out parameter).
    * \param config Configuration of the participant passed as JSON string
    * \param participantName Name of the participant
    * \param cDomainId ID of the domain/simulation to join
    *
    */
    CIntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_create(ib_SimulationParticipant** outParticipant, const char* cJsonConfig, const char* cParticipantName, const char* cDomainId);
    
    typedef ib_ReturnCode (*ib_SimulationParticipant_destroy_t)(ib_SimulationParticipant* self);
    /*! \brief Destroy a simulation participant and its associated simulation elements.
    *
    * Destroys the simulation participant and its created simulation elements such as e.g. Can controllers.
    *
    * \param self The simulation participant to be destroyed.
    *
    */
    CIntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_destroy(ib_SimulationParticipant* self);

#ifdef __cplusplus
}
#endif
