=======
VIB API
=======
.. |Participant| replace:: :doc:`Participant<participant>`
.. |header| replace:: :doc:`header<header>`

This document describes the user available programming interfaces of the Vector
Integration Bus (VIB). If you are not yet familiar with the VIB, have a look at
the :doc:`Quick Start<../usage/quickstart>`.


VIB Entry Point and API Organization
------------------------------------

The Participant constitutes the main entry point into the Integration Bus API.

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

Data Messages, Generic Messages, Rpc and IO
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These services do not model a real-world bus and can be used for
generic programming and interoperability tasks.

.. toctree::
  :maxdepth: 1
  
  datamessage
  genericmessage
  rpc
  io

State Handling and Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following services provide time synchronization and orchestrated state
management.

.. toctree::
  :maxdepth: 1

  participantcontroller
  systemcontroller
  systemmonitor

Experimental C API
------------------------------------
The Integration Bus API contains an experimental C API, that enables C 
developers to integrate the Integration Bus directly into their
applications.

.. admonition:: Warning:

   Currently, the state of the C API is experimental and its feature set is not
   yet feature complete.

.. toctree::
  :maxdepth: 1

  capi/capi

Developer Documentation
-----------------------
The following documentation is intended for developers working on the 
Integration Bus itself.

.. toctree::
   :maxdepth: 1

   Building from Source <../development/build>
   Writing Documentation<../development/rst-help>
