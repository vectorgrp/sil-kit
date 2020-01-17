=======
VIB API
=======
.. |ComAdapter| replace:: :doc:`ComAdapter<comadapter>`
.. |header| replace:: :doc:`header<header>`

.. _sec:api-services:

This document describes the user available programming interfaces of the Vector
Integration Bus (VIB).
If you are not yet familiar with the VIB, have a look at the :doc:`Quick Start<../usage/quickstart>`.


The following is a list of supported VIB API, services and vehicular networks.
The main entry points into the Integration Bus API are discussed in |ComAdapter|
and in the |header| documents.

User API
--------
The main programming interfaces for configuration and  accessing of the
Integration Bus services and controllers are listed below.

.. toctree::
   :maxdepth: 1
   
   header
   config
   comadapter

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

  io
  genericmessage

Advanced Programming Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following sections contain the advanced programming interfaces for
state and system handling.

.. toctree::
  :maxdepth: 1

  participantcontroller
  The System Controller <synchronisation>
  The System Monitor <systemmonitor>

Developer Documentation
-----------------------
The following documentation is intended for developers working on the 
Integration Bus itself.

.. toctree::
   :maxdepth: 1

   Building from Source <../development/build>
   Writing Documentation<../development/rst-help>
