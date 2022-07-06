=======
API
=======
.. |Participant| replace:: :doc:`Participant<participant>`
.. |header| replace:: :doc:`header<header>`


This document describes the user available programming interfaces of the Vector
SilKit (SILKIT). If you are not yet familiar with the SILKIT, have a look at
the :doc:`Quick Start<../usage/quickstart>`.


API Entry Point and API Organization
------------------------------------

The Participant constitutes the main entry point into the SilKit API.

.. toctree::
   :maxdepth: 1
   
   participant
   header

.. _sec:api-services:

Services
--------
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

Data Messages and Rpc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These services do not model a real-world bus and can be used for
generic programming and interoperability tasks.

.. toctree::
  :maxdepth: 1
  
  datamessage
  rpc

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

C-API
------------------------------------

The Vector SilKit provides an additional C API, that allows to use the Vector SilKit directly
with the C programming language:

.. toctree::
   :maxdepth: 1
   
   capi/capi
