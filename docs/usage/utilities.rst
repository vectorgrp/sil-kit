==============
VIB Utilities
==============

.. contents::
   :local:
   :depth: 1

Running a Vector Integration Bus (VIB) system is supported by several utilities.
The launcher's purpose is to simplify starting ensembles  of participants
and other simulation utilities from a given configuration file.
The registry is a mandatory part of the VAsio middleware -- it implements
connection and service discovery for participants.
The system monitor and controller are provided for convenience. They implement
a simulation-wide state tracking and system command handling which is required
in every simulation. However, using these processes is not mandatory -- users
of the VIB are free to implement their own system and state handling.

.. _sec:util-launcher:

Launcher
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The Launcher script allows starting multiple participants of a
         VIB system.
         Nevertheless, all participants, e.g. the Demos, may be executed
         manually as described in the corresponding section.
   *  -  Source location
      -  ``Launcher``
   *  -  Requirements
      -  * `Python <https://www.python.org/downloads/>`_  v3.x+
         * Adaptation of the :doc:`LaunchConfigurations <../configuration/launch-configurations>` section in the
           Integration Bus config (e.g. Demos/Can/IbConfig_DemoCan.json).
         * Adaptation of INTEGRATIONBUS_BINPATH & INTEGRATIONBUS_LIBPATH
           in process environment. Default is to
           infer the IntegrationBus paths from the path of IbLauncher.py.
   *  -  Parameters
      -  There are eight arguments:

         #. Filename of the IB Configuration to be used (IB config file).
         #. Launch configuration ``[-c] CONFIG`` (e.g. Installation/Developer-WinDebug/Developer-WinRelease/Developer-Linux)
         #. Network node ``[-n NODE]``, optional
         #. Domain ID (optional); defaults to ``42``.
         #. Command ``[-x COMMAND]`` (e.g. setup/run/teardown/setup-run-teardown(default)), optional
         #. Logfile ``[-l LOGFILE]``, optional
         #. Retries ``[-r RETRIES]``, optional
         #. Quiet execution ``[-q]``, optional
   *  -  Usage Example
      -  .. code-block:: powershell

            # Launch CAN demo w/o Network Simulator VIBE:
            IbLauncher.py Demos/Can/IbConfig_DemoCan.json -c Installation

   *  -  Notes
      -  * The distribution package contains the launcher in the
           ``Integrationbus/bin/`` directory.
         * INTEGRATIONBUS_BINPATH & INTEGRATIONBUS_LIBPATH may be defined
           as environment variables.


.. _sec:util-registry:

VAsio Registry
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - The Registry enables discovery between IB participants when using the
        VAsio middleware. It is mandatory, when using the VAsio middleware.

   *  - Source location
      - ``Utilities/IbRegistry``
   *  - Requirements
      - None
   *  - Parameters
      - There are up to three positional arguments:

        #. Filename of the IB Configuration to be used (IB config file).
        #. Domain ID (optional); defaults to ``42``.
        #. ``--use-signal-handler`` (optional); Uses a signal handler for shutdown and does not read from stdin.

   *  - Usage Example
      - .. code-block:: powershell

            # Start the IbRegistry using the CAN demo configuration
            IbRegistry Demos/Can/IbConfig_DemoCan.json 42

   *  - Notes
      -  * The distribution package contains the IbRegistry in the
           ``Integrationbus-NonRedistributable/`` directory of the distribution.
         * When using the VAsio middleware, the IbRegistry must be started
           before the IB participants. When using the Launcher, the IbRegistry
           is automatically started if the IbConfig specifies VAsio as the
           :doc:`active middleware<../configuration/middleware-configuration>`.

.. versionchanged:: >3.4.0
   
    The IbRegistry was moved from ``IntegrationBus/bin`` to ``IntegrationBus-NonRedistributable``
    directory in the distribution, where the shared library resides.

.. _sec:util-system-controller:

SystemController
~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemController implements state handling for the participants of
         an Integration Bus system.
         Examples for state change commands called by the SystemController are
         'Run' or 'Stop'.
   *  -  Source location
      -  ``Utilities/IbSystemController``
   *  -  Requirements
      -  The SystemController requires an established Integration Bus System.
         Thus, it has to be started after other (active) participants.
   *  -  Parameters
      -  There are up to two positional argument:

         #. Filename of the IB Configuration to be used (IB config file).
         #. Domain ID (optional); defaults to ``42``.
   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemController for CAN Demo w/o Network Simulator VIBE:
            IbSystemController Demos/Can/IbConfig_DemoCan.json
   *  -  Notes
      -  * The distribution package contains the IbSystemController in the
           ``Integrationbus/bin/`` directory.



.. _sec:util-system-controller-interactive:

SystemControllerInteractive
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  This variant of the system controller allows setting the system states
         manually via a command line interface. A user can enter commands on
         standard input, e.g. "Run", "Stop", "Shutdown".
   *  -  Source location
      -  ``Utilities/IbSystemControllerInteractive``
   *  -  Requirements
      -  The SystemControllerInteractive requires an established Integration Bus
         System.
         Thus, it has to be started after other (active) participants.
   *  -  Parameters
      -  There are up to two positional argument:

         #. Filename of the IB Configuration to be used (IB config file).
         #. Domain ID (optional); defaults to ``42``.
   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemControllerInteractive for CAN Demo w/o Network Simulator VIBE:
            IbSystemControllerInteractive Demos/Can/IbConfig_DemoCan.json
   *  -  Notes
      -  * The distribution package contains the IbSystemControllerInteractive
           in the ``Integrationbus/bin/`` directory.


.. _sec:util-system-monitor:

SystemMonitor
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemMonitor visualizes the states of the participants of an
         Integration Bus system.
   *  -  Source location
      -  ``Utilities/IbSystemMonitor``
   *  -  Requirements
      -  None
   *  -  Parameters
      -  There are up to two positional arguments:
          
         #. Filename of the IB Configuration to be used (IB config file).
         #. Domain ID (optional); defaults to ``42``.
   *  -  Usage Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor for CAN Demo w/o Network Simulator VIBE:
            IbSystemMonitor Demos/Can/IbConfig_DemoCan.json
   *  -  Notes
      -  * The distribution package contains the IbSystemMonitor in the
           ``Integrationbus/bin/`` directory.
         * The SystemMonitor represents a passive participant in an Integration
           Bus system. Thus, it can be (re)started at any time.
