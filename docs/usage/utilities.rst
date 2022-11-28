==============
Utilities
==============

.. contents::
   :local:
   :depth: 1

Running a Vector SIL Kit system is supported by several utilities.
The registry is a mandatory part of the SIL Kit integrated middleware -- it is needed to establish the connections between simulation participants at the start of a simulation.
The system monitor and controller are provided for convenience. They implement
a simulation-wide state tracking and system command handling which is required
in every simulation. However, using these utilities is not mandatory -- users
of the SIL Kit are free to implement their own system and state handling.

.. _sec:util-registry:

sil-kit-registry
~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - The Registry enables discovery between SIL Kit participants. It is needed for Vector SIL Kit simulations.

   *  - Source location
      - ``Utilities/SilKitRegistry``
   *  - Requirements
      - None
   *  - Parameters
      - -v, --version                        Get version info.
        -h, --help                           Show the help of the SIL Kit Registry.
        -s, --use-signal-handler             Exit this process when a signal is received. If not set, the process runs infinitely.
        -u, --listen-uri <silkit-uri>        The silkit:// URI the registry should listen on. Defaults to 'silkit://localhost:8500'.
        -l, --log <level>                    Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.
        -g, --generate-configuration <path>  Path and filename of a participant configuration file to generate containing the URI the registry is using.
        -d, --dashboard-uri <dashboard-uri>  The http:// URI the data should be sent to. Defaults to 'http://localhost:8082'.

   *  - Usage Example
      - .. code-block:: powershell

           # Start the SIL Kit Registry
           sil-kit-registry
   *  - Notes
      -  * The SIL Kit Registry is packaged in the ``SilKit/bin`` directory.
         * The SIL Kit Registry must be started before other SIL Kit participants,
           either with this process or using the :cpp:func:`StartListening()<SilKit::Vendor::Vector::ISilKitRegistry::StartListening()>` API.
         * When the port ``0`` is specified in the URI (``--listen-uri``) the operating system will choose a random port for the registry.
           This port is used in the generated configuration file (``--generate-configuration``) for use in CI environments.
         * The registry will run if either binding to the TCP socket, or the Domain socket, or both succeeds.
           If only TCP or Domain sockets are used, because one of the bindings failed for some reason, a warning will be logged.
           It will exit with an error if neither is available.
         * The SIL Kit Dashboard is experimental and might be changed or removed in future versions of the SIL Kit.


.. _sec:util-system-controller:

sil-kit-system-controller
~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The sil-kit-system-controller implements state handling for the participants of
         an SIL Kit system.
         Examples for state change commands called by the SystemController are
         'Run' or 'Stop'.
   *  -  Source location
      -  ``Utilities/SilKitSystemController``
   *  -  Requirements
      -  The sil-kit-system-controller needs a running sil-kit-registry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of sil-kit-system-controller.
         -u, --connect-uri <silkitUri>                The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration <configuration>          Path and filename of the participant configuration YAML or JSON file.
         -ni, --non-interactive                       Run without awaiting any user interactions at any time.
         
         | **<participantName1>, <participantName2> ...**
         |  Names of participants to wait for before starting simulation.
   
   *  -  Usage Example
      -  .. code-block:: powershell

            # Start sil-kit-system-controller process and wait for Participant1 and Participant2:
            sil-kit-system-controller Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the sil-kit-system-controller in the
           ``SilKit/bin/`` directory.


.. _sec:util-monitor:

sil-kit-monitor
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The sil-kit-monitor visualizes the states of the participants of a
         SIL Kit simulation.
   *  -  Source location
      -  ``Utilities/SilKitMonitor``
   *  -  Requirements
      -  Requires a running sil-kit-registry to connect to.
   *  -  Parameters
      -  -v, --version                           Get version info.
         -h, --help                              Show the help of the sil-kit-monitor.
         -u, --connect-uri <silkitUri>           The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>            The participant name used to take part in the simulation. Defaults to 'SystemMonitor'.
         -c, --configuration  <configuration>    Path and filename of the participant configuration YAML or JSON file.

   *  -  Usage Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor
            sil-kit-monitor
   *  -  Notes
      -  * The distribution package contains the sil-kit-monitor in the ``SilKit/bin/`` directory.
         * The sil-kit-monitor represents a passive participant in a SIL Kit system. Thus, it can be (re)started at any time.
