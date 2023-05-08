.. _sec:capi:

================================
Entry Point and API Organization
================================

.. contents::
   :local:
   :depth: 3


.. highlight:: c

SIL Kit Entry Point and API Organization
========================================

The main entry point of the C API is the function to obtain a ``SilKit_Participant``::

    SilKit_ParticipantConfiguration* participantConfiguration = NULL;
    SilKit_Participant* participant = NULL;
    const char* configString = "{ ... }";
    const char* participantName = "CanWriter";
    const char* registryUri = "silkit://localhost:8500";
    SilKit_ReturnCode result;
    result = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, configString);
    if (result != SilKit_ReturnCode_SUCCESS) { ... }
    result = SilKit_Participant_Create(&participant, configString,
                                                            participantName, registryUri);
    if (result != SilKit_ReturnCode_SUCCESS) { ... }

All further services of the C API of the SIL Kit are requested through this Participant.

Entities obtained through the Participant must not be destroyed/deleted by the user of the API.
All entities, that are provided through the API expect for the ``SilKit_Participant`` are destroyed through
the internals of the SIL Kit implementation.

After creation of a Participant it must be ensured that eventually ``SilKit_Participant_Destroy`` is called
with the corresponding pointer to the ``SilKit_Participant`` entity.


API and Data Type Reference
===========================

General API
-----------
.. doxygenfunction:: SilKit_ReturnCodeToString
.. doxygenfunction:: SilKit_GetLastErrorString

Participant API
---------------

.. doxygenfunction:: SilKit_Participant_Create
.. doxygenfunction:: SilKit_Participant_Destroy

Most creator functions for other objects (such as bus controllers) require a ``SilKit_Participant``, 
which is the factory object, as input parameter.

Logger API 
----------

The Logger API can be used to write log messages.

.. doxygenfunction:: SilKit_Participant_GetLogger
.. doxygenfunction:: SilKit_Logger_Log

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_LoggingLevel
