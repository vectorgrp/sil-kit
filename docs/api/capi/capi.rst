.. _sec:capi:

=====
C API
=====

.. contents::
   :local:
   :depth: 3


.. highlight:: c

VIB Entry Point and API Organization
====================================

The main entry point of the C API is the function to obtain a ib_Participant::

    ib_Participant* participant = NULL;
    const char* jsonString = "{ ... }";
    const char* participantName = "CanWriter";
    const char* domainId = "1";
    ib_ReturnCode result = ib_Participant_Create(&participant, jsonString,
                                                            participantName, domainId, ib_False);

All further services of the C API of the IntegrationBus are requested through this Participant.

Entities obtained through the Participant must not be destroyed/deleted by the user of the API.
All entities, that are provided through the API expect for the ib_Participant are destroyed through
the internals of the Integration Bus implementation.

After creation of a Participant it must be ensured that eventually ib_Participant_Destroy is called
with the corresponding pointer to the ib_Participant entity.


API and Data Type Reference
===========================

General API
-----------
.. doxygenfunction:: ib_ReturnCodeToString
.. doxygenfunction:: ib_GetLastErrorString

Participant API
---------------

.. doxygenfunction:: ib_Participant_Create
.. doxygenfunction:: ib_Participant_Destroy

Most creator functions for other objects (such as bus controllers) require an ib_Participant, 
which is the factory object, as input parameter.

Services
--------

Vehicle Network Controller
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
  :maxdepth: 1

  can
  ethernet
  lin
  flexray

Data Messages and RPC
~~~~~~~~~~~~~~~~~~~~~

.. toctree::
  :maxdepth: 1

  data
  rpc

State Handling and Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
  :maxdepth: 1

  participantcontroller
  systemcontroller
  systemmonitor


The Logger API 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The Logger API can be used to write log messages.

.. doxygenfunction:: ib_Participant_GetLogger
.. doxygenfunction:: ib_Logger_Log

Enumerations and Typedefs
-------------------------
.. doxygentypedef:: ib_LoggingLevel