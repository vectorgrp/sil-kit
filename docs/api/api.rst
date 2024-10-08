.. include:: /substitutions.rst

=========
C/C++ API
=========

This document describes the user available programming interfaces of the Vector |ProductName|.
If you are not yet familiar with the |ProductName|, have a look at the :doc:`Developers Guide<../for-developers/developers>`.
Information on API organization in the |ProductName|:

.. toctree::
   :maxdepth: 1

   header
   memory-management

C++ API
-------

Participant
~~~~~~~~~~~

The Participant constitutes the main entry point into the |ProductName| API.

.. toctree::
   :maxdepth: 1
   
   participant

.. _sec:api-services:

Services
~~~~~~~~

The services API includes common data types, supports various vehicular networks, and provides generic publish/subscribe and RPC facilities.

Logging
+++++++

The Logging service can be used for writing log messages of specified log levels to various types of sinks.

.. toctree::
   :maxdepth: 1

   services/logging

Vehicle Network Controller
++++++++++++++++++++++++++

Several different vehicular networks are supported by corresponding controller interfaces.

.. toctree::
  :maxdepth: 1

  services/can
  services/lin
  services/flexray
  services/ethernet

Data Publish/Subscribe and RPC
++++++++++++++++++++++++++++++

These services do not model a real-world bus and can be used for generic programming and interoperability tasks.

.. toctree::
  :maxdepth: 1

  services/pubsub
  services/rpc

|ProductName| provides a recommended default serialization/deserialization API for Data Publish/Subscribe and RPC:

.. toctree::
  :maxdepth: 1

  serdes

State Handling and Synchronization
++++++++++++++++++++++++++++++++++

The following services provide time synchronization and orchestrated state management.

.. toctree::
  :maxdepth: 1

  services/lifecycle
  services/timesync


System Utilities
++++++++++++++++

These system utilities can be used to control and monitor the simulation as a whole.

.. toctree::
  :maxdepth: 1

  system-utilities/systemcontroller
  system-utilities/systemmonitor

Common Data Types
+++++++++++++++++

Some data types are shared between services of the |ProductName|.

.. toctree::
  :maxdepth: 1

  common-data-types

Custom Network Simulator
++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  netsim


C API
-----

The |ProductName| provides an additional API that allows to use the |ProductName| directly
with the C programming language:

.. toctree::
  :maxdepth: 1

  capi/capi-main

Services
~~~~~~~~

Vehicle Network Controller
++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi/capi-can
  capi/capi-ethernet
  capi/capi-lin
  capi/capi-flexray

Data Publish/Subscribe and RPC
++++++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi/capi-data
  capi/capi-rpc

State Handling and Synchronization
++++++++++++++++++++++++++++++++++

.. toctree::
  :maxdepth: 1

  capi/capi-orchestration
  capi/capi-systemmonitor
