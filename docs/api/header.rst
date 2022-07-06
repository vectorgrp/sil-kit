============
Header Files
============
The header files are organized hierarchically starting with the ``silkit`` directory.
The subdirectory structure resembles the layered architecture of the SILKIT.
As a rule of thumb, each component has a subdirectory with its own namespace:

.. list-table:: Include Directories
    :widths: 15 15 70
    :header-rows: 1

    * - Path
      - Namespace
      - Description
    * - silkit/
      - ``silkit``
      - Main header, version and macros.
    * - silkit/mw
      - ``SilKit::Core``
      - :doc:`Middleware<../configuration/middleware-configuration>` specific interfaces, data types and exceptions.
    * - silkit/mw/sync
      - ``SilKit::Services::Orchestration``
      - Synchronization mechanisms.
    * - silkit/sim
      - ``SilKit::Services``
      - :ref:`Simulation and Service<sec:api-services>` specific definitions.
    * - silkit/util
      - ``SilKit::Util``
      - Generic programming utilities
    * - silkit/cfg
      - ``SilKit::Config``
      - Data structures representing the SILKIT :doc:`../configuration/configuration`.

The subdirectory layout follows mostly a uniform scheme for substantial SILKIT components:
 - `silkit/<component>/all.hpp` is sufficient to use the component.
 - `silkit/<component>/fwd_decl.hpp` contains forward declarations. 
 - Datatypes are declared in ``silkit/<component>/<Component>Datatypes.hpp``.


.. _sec:header-vib-main:

Using the SILKIT Headers
---------------------
The main header file is ``silkit/SilKit.hpp`` which defines the 
:cpp:func:`CreateParticipant()<SilKit::CreateParticipant()>` function.
Version information can be retrieved using the ``silkit/version.hpp`` header 
and its functions, e.g. by using the 
:cpp:func:`Version::String()<SilKit::Version::String()>` function.

Further Reading
---------------
Refer to the :doc:`API overview<api>` for a description of the available
services and interfaces.
