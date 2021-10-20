===================
Experimental C API
===================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the Experimental C API
-------------------------

VIB Entry Point and API Organization
~~~~~~~~~~~~~~~~~~~~

The main entry point of the C API is the function to obtain a ib_SimulationParticipant*::

    ib_SimulationParticipant* participant;
    const char* jsonString = "{ ... }";
    const char* participantName = "CanWriter";
    const char* domainId = "1";
    ib_ReturnCode = ib_SimulationParticipant_create(&participant, jsonString, participantName, domainId);

All further services of the C API of the IntegrationBus are requested through this SimulationParticipant.

Entities obtained through the SimulationParticipant must not be destroyed/deleted by the user of the API.
All entities, that are provided through the API expect for the ib_SimulationParticipant are destroyed through
the internals of the Integration Bus implementation.

After creation of a SimulationParticipant it must be ensured that eventually ib_Participant_destroy is called
with the corresponding pointer to the ib_SimulationParticipant entity.


API and Data Type Reference
--------------------------------------------------
General API
~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_SimulationParticipant_create
.. doxygenfunction:: ib_SimulationParticipant_destroy
.. doxygenfunction:: ib_ReturnCodeToString
.. doxygenfunction:: ib_GetLastErrorString

Can API
~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_CanController_create
.. doxygenfunction:: ib_CanController_Start
.. doxygenfunction:: ib_CanController_Stop
.. doxygenfunction:: ib_CanController_Reset
.. doxygenfunction:: ib_CanController_Sleep
.. doxygenfunction:: ib_CanController_SendFrame
.. doxygenfunction:: ib_CanController_SetBaudRate
.. doxygenfunction:: ib_CanController_RegisterTransmitStatusHandler
.. doxygenfunction:: ib_CanController_RegisterReceiveMessageHandler
.. doxygenfunction:: ib_CanController_RegisterStateChangedHandler
.. doxygenfunction:: ib_CanController_RegisterErrorStateChangedHandler


   
