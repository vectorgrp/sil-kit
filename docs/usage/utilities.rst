==============
Utilities
==============

.. contents::
   :local:
   :depth: 1

Running a Vector SilKit system is supported by several utilities.
The registry is a mandatory part of the VAsio middleware -- it implements
connection and service discovery for participants.
The system monitor and controller are provided for convenience. They implement
a simulation-wide state tracking and system command handling which is required
in every simulation. However, using these utilities is not mandatory -- users
of the SILKIT are free to implement their own system and state handling.

.. _sec:util-registry:

SilKitRegistry
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - The Registry enables discovery between SilKit participants when using the
        VAsio middleware. It is mandatory when using the VAsio middleware.

   *  - Source location
      - ``Utilities/SilKitRegistry``
   *  - Requirements
      - None
   *  - Parameters
      - -v, --version                         Get version info.
        -h, --help                            Show the help of the SilKitRegistry.
        -s, --use-signal-handler              Exit this process when a signal is received. If not set, the process runs infinitely.
        -u, --listen-uri <silkit-uri>         The silkit:// URI the registry should listen on. Defaults to 'silkit://localhost:8500'.
        -l, --log <level>                     Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.
        -c, --configuration <configuration>   Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

   *  - Usage Example
      - .. code-block:: powershell

            # Start the SilKitRegistry
            SilKitRegistry

   *  - Notes
      -  * The SilKitRegistry is packaged in the ``SilKit/bin`` directory.
         * The SilKitRegistry must be started before other SilKit participants,
           either with this process or using the :cpp:func:`ProvideDomain()<SilKit::Vendor::Vector::ISilKitRegistry::ProvideDomain()>` API.


.. _sec:util-system-controller:

SilKitSystemController
~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemController implements state handling for the participants of
         an SilKit system.
         Examples for state change commands called by the SystemController are
         'Run' or 'Stop'.
   *  -  Source location
      -  ``Utilities/SilKitSystemController``
   *  -  Requirements
      -  The SystemController needs a running SilKitRegistry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of the SilKitSystemController.
         -u, --connect-uri <silkitUri>                   The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration <configuration>          Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

         <participantName1>, <participantName2>, ...  Names of participants to wait for before starting simulation.

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemController and wait for Participant1 and Participant2:
            SilKitSystemController Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the SilKitSystemController in the
           ``SilKit/bin/`` directory.



.. _sec:util-system-controller-interactive:

SilKitSystemControllerInteractive
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  This variant of the system controller allows sending system commands
         manually via a command line interface. A user can enter commands on
         standard input: "Run", "Stop", "Abort", "Shutdown <ParticipantName>, Restart <ParticipantName>"
   *  -  Source location
      -  ``Utilities/SilKitSystemControllerInteractive``
   *  -  Requirements
      -  The SystemController needs a running SilKitRegistry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of the SilKitSystemControllerInteractive.
         -u, --connect-uri <silkitUri>                   The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration  <configuration>         Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

         <participantName1>, <participantName2>, ...  Names of participants that are required for the simulation (e.g. synchronized paricipants).

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemControllerInteractive for two participants:
            SilKitSystemControllerInteractive Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the SilKitSystemControllerInteractive
           in the ``SilKit/bin/`` directory.


.. _sec:util-system-monitor:

SilKitSystemMonitor
~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemMonitor visualizes the states of the participants of an
         SilKit simulation.
   *  -  Source location
      -  ``Utilities/SilKitSystemMonitor``
   *  -  Requirements
      -  Requires a running SilKitRegistry to connect to.
   *  -  Parameters
      -  -v, --version                           Get version info.
         -h, --help                              Show the help of the SilKitSystemMonitor.
         -u, --connect-uri <silkitUri>              The registry URI to connect to. Defaults to 'silkit://localhost:8500'.
         -n, --name <participantName>            The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration  <configuration>    Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

   *  -  Usage Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor
            SilKitSystemMonitor
   *  -  Notes
      -  * The distribution package contains the SilKitSystemMonitor in the ``SilKit/bin/`` directory.
         * The SystemMonitor represents a passive participant in an SIL Kit system. Thus, it can be (re)started at any time.
