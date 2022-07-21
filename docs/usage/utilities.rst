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
      - -v, --version                         Get version info.
        -h, --help                            Show the help of the SIL Kit Registry.
        -s, --use-signal-handler              Exit this process when a signal is received. If not set, the process runs infinitely.
        -u, --listen-uri <silkit-uri>         The silkit:// URI the registry should listen on. Defaults to 'silkit://localhost:8500'.
        -l, --log <level>                     Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.
        -c, --configuration <configuration>   Path and filename of the participant configuration YAML or JSON file.

   *  - Usage Example
      - .. code-block:: powershell

            # Start the SIL Kit Registry
            sil-kit-registry

   *  - Notes
      -  * The SIL Kit Registry is packaged in the ``SilKit/bin`` directory.
         * The SIL Kit Registry must be started before other SIL Kit participants,
           either with this process or using the :cpp:func:`ProvideDomain()<SilKit::Vendor::Vector::ISilKitRegistry::ProvideDomain()>` API.


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

         <participantName1>, <participantName2>, ...  Names of participants to wait for before starting simulation.

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start sil-kit-system-controller process and wait for Participant1 and Participant2:
            sil-kit-system-controller Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the sil-kit-system-controller in the
           ``SilKit/bin/`` directory.



.. _sec:util-system-controller-interactive:

sil-kit-system-controller-interactive
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  This variant of the sil-kit-system-controller allows sending system commands
         manually via a command line interface. A user can enter commands on
         standard input: "Run", "Stop", "Abort", "Shutdown <ParticipantName>, Restart <ParticipantName>"
   *  -  Source location
      -  ``Utilities/SilKitSystemControllerInteractive``
   *  -  Requirements
      -  The sil-kit-system-controller needs a running sil-kit-registry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of sil-kit-system-controller-interactive.
         -u, --connect-uri <silkitUri>                The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration  <configuration>         Path and filename of the participant configuration YAML or JSON file.

         <participantName1>, <participantName2>, ...  Names of participants that are required for the simulation (e.g., synchronized paricipants).

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start sil-kit-system-controller-interactive process for two participants:
            sil-kit-system-controller-interactive Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the sil-kit-system-controller-interactive
           in the ``SilKit/bin/`` directory.


.. _sec:util-system-monitor:

sil-kit-system-monitor
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The sil-kit-system-monitor visualizes the states of the participants of a
         SIL Kit simulation.
   *  -  Source location
      -  ``Utilities/SilKitSystemMonitor``
   *  -  Requirements
      -  Requires a running sil-kit-registry to connect to.
   *  -  Parameters
      -  -v, --version                           Get version info.
         -h, --help                              Show the help of the sil-kit-system-monitor.
         -u, --connect-uri <silkitUri>           The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>            The participant name used to take part in the simulation. Defaults to 'SystemMonitor'.
         -c, --configuration  <configuration>    Path and filename of the participant configuration YAML or JSON file.

   *  -  Usage Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor
            sil-kit-system-monitor
   *  -  Notes
      -  * The distribution package contains the sil-kit-system-monitor in the ``SilKit/bin/`` directory.
         * The sil-kit-system-monitor represents a passive participant in a SIL Kit system. Thus, it can be (re)started at any time.
