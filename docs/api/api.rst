===
API
===
.. |Participant| replace:: :doc:`Participant<participant>`
.. |header| replace:: :doc:`header<header>`


This document describes the user available programming interfaces of the Vector
SIL Kit (SIL Kit). If you are not yet familiar with the SIL Kit, have a look at
the :doc:`Developers Guide<../for-developers/developers>`.


API Entry Point and API Organization
------------------------------------

The Participant constitutes the main entry point into the SIL Kit API.

.. toctree::
   :maxdepth: 1
   
   participant
   header
   memory-management
   
.. _sec:api-services:

Services
--------

The services API includes common data types, supports various vehicular networks, and provides generic publish/subscribe and RPC facilities.

.. toctree::
   :maxdepth: 1
   
   logging

Vehicle Network Controller
~~~~~~~~~~~~~~~~~~~~~~~~~~
Several different vehicular networks are supported by corresponding
controller interfaces.

.. toctree::
  :maxdepth: 1

  can
  lin
  flexray
  ethernet

Data Publish/Subscribe and RPC
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These services do not model a real-world bus and can be used for
generic programming and interoperability tasks.

.. toctree::
  :maxdepth: 1

  pubsub
  rpc

SIL Kit provides a recommended default serialization/deserialization API for Data Publish/Subscribe and RPC:

.. toctree::
  :maxdepth: 1

  serdes

State Handling and Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following services provide time synchronization and orchestrated state
management.

.. toctree::
  :maxdepth: 1

  lifecycleService
  timeSyncService
  systemcontroller
  systemmonitor

Common Data Types
~~~~~~~~~~~~~~~~~

.. toctree::
  :maxdepth: 1

  common

C API
-----

The SIL Kit provides an additional API that allows to use the SIL Kit directly
with the C programming language:

.. toctree::
  :maxdepth: 1

  capi-main

Services
~~~~~~~~~~~~~~~~~~~~~~~~~~

Vehicle Network Controller
++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi-can
  capi-ethernet
  capi-lin
  capi-flexray

Data Publish/Subscribe and RPC
++++++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi-data
  capi-rpc

State Handling and Synchronization
++++++++++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi-orchestration
  capi-systemmonitor
