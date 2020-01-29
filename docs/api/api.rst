=======
VIB API
=======
.. |ComAdapter| replace:: :doc:`ComAdapter<comadapter>`
.. |header| replace:: :doc:`header<header>`

.. _sec:api-services:

This document describes the user available programming interfaces of the Vector
Integration Bus (VIB). If you are not yet familiar with the VIB, have a look at
the :doc:`Quick Start<../usage/quickstart>`.


VIB Entry Point and API Organization
------------------------------------

The ComAdapter constitutes the main entry point into the Integration Bus API.

.. toctree::
   :maxdepth: 1
   
   comadapter
   header

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

Generic Messages and IO
~~~~~~~~~~~~~~~~~~~~~~~~~~
These services do not model a real-world bus and can be used for
generic programming and interoperability tasks.

.. toctree::
  :maxdepth: 1

  genericmessage
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


Developer Documentation
-----------------------
The following documentation is intended for developers working on the 
Integration Bus itself.

.. toctree::
   :maxdepth: 1

   Building from Source <../development/build>
   Writing Documentation<../development/rst-help>
