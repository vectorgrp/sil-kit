============
Header Files
============
The header files are organized hierarchically starting with the ``ib`` directory.
The subdirectory structure resembles the layered architecture of the VIB.
As a rule of thumb, each component has a subdirectory with its own namespace:

.. list-table:: Include Directories
    :widths: 15 15 70
    :header-rows: 1

    * - Path
      - Namespace
      - Description
    * - ib/
      - ``ib``
      - Main header, version and macros.
    * - ib/mw
      - ``ib::mw``
      - :doc:`Middleware<../configuration/middleware-configuration>` specific interfaces, data types and exceptions.
    * - ib/mw/sync
      - ``ib::mw::sync``
      - Synchronization mechanisms.
    * - ib/sim
      - ``ib::sim``
      - :ref:`Simulation and Service<sec:api-services>` specific definitions.
    * - ib/util
      - ``ib::util``
      - Generic programming utilities
    * - ib/cfg
      - ``ib::cfg``
      - Data structures representing the VIB :doc:`../configuration/configuration`.

The subdirectory layout follows mostly a uniform scheme for substantial VIB components:
 - `ib/<component>/all.hpp` is sufficient to use the component.
 - `ib/<component>/fwd_decl.hpp` contains forward declarations. 
 - Datatypes are declared in ``ib/<component>/<Component>Datatypes.hpp``.


.. _sec:header-vib-main:

Using the VIB Headers
---------------------
The main header file is ``ib/IntegrationBus.hpp`` which defines the 
:cpp:func:`CreateParticipant()<ib::CreateParticipant()>` function.
Version information can be retrieved using the ``ib/version.hpp`` header 
and its functions, e.g. by using the 
:cpp:func:`version::String()<ib::version::String()>` function.

Further Reading
---------------
Refer to the :doc:`API overview<api>` for a description of the available
services and interfaces.
