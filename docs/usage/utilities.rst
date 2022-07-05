==============
Utilities
==============

.. contents::
   :local:
   :depth: 1

Running a Vector Integration Bus (VIB) system is supported by several utilities.
The registry is a mandatory part of the VAsio middleware -- it implements
connection and service discovery for participants.
The system monitor and controller are provided for convenience. They implement
a simulation-wide state tracking and system command handling which is required
in every simulation. However, using these utilities is not mandatory -- users
of the VIB are free to implement their own system and state handling.

.. _sec:util-registry:

IbRegistry
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - The Registry enables discovery between IB participants when using the
        VAsio middleware. It is mandatory when using the VAsio middleware.

   *  - Source location
      - ``Utilities/IbRegistry``
   *  - Requirements
      - None
   *  - Parameters
      - -v, --version                         Get version info.
        -h, --help                            Show the help of the IbRegistry.
        -s, --use-signal-handler              Exit this process when a signal is received. If not set, the process runs infinitely.
        -u, --listen-uri <vib-uri>            The vib:// URI the registry should listen on. Defaults to 'vib://localhost:8500'.
        -l, --log <level>                     Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.
        -c, --configuration <configuration>   Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

   *  - Usage Example
      - .. code-block:: powershell

            # Start the IbRegistry
            IbRegistry

   *  - Notes
      -  * The IbRegistry is packaged in the ``IntegrationBus/bin`` directory.
         * The IbRegistry must be started before other IB participants,
           either with this process or using the :cpp:func:`ProvideDomain()<ib::vendor::vector::IIbRegistry::ProvideDomain()>` API.


.. _sec:util-system-controller:

IbSystemController
~~~~~~~~~~~~~~~~~~

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
      -  The SystemController needs a running IbRegistry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of the IbSystemController.
         -u, --connect-uri <vibUri>                   The registry URI to connect to. Defaults to 'vib://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration <configuration>          Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

         <participantName1>, <participantName2>, ...  Names of participants to wait for before starting simulation.

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemController and wait for Participant1 and Participant2:
            IbSystemController Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the IbSystemController in the
           ``Integrationbus/bin/`` directory.



.. _sec:util-system-controller-interactive:

IbSystemControllerInteractive
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  This variant of the system controller allows sending system commands
         manually via a command line interface. A user can enter commands on
         standard input: "Run", "Stop", "Abort", "Shutdown <ParticipantName>, Restart <ParticipantName>"
   *  -  Source location
      -  ``Utilities/IbSystemControllerInteractive``
   *  -  Requirements
      -  The SystemController needs a running IbRegistry to connect to. 
         Furthermore, it requires a list of synchronized participants that are needed to start the simulation as input.
   *  -  Parameters
      -  -v, --version                                Get version info.
         -h, --help                                   Show the help of the IbSystemControllerInteractive.
         -u, --connect-uri <vibUri>                   The registry URI to connect to. Defaults to 'vib://localhost:8500'.
         -n, --name <participantName>                 The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration  <configuration>         Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

         <participantName1>, <participantName2>, ...  Names of participants that are required for the simulation (e.g. synchronized paricipants).

   *  -  Usage Example
      -  .. code-block:: powershell

            # Start SystemControllerInteractive for two participants:
            IbSystemControllerInteractive Participant1 Participant2
   *  -  Notes
      -  * The distribution package contains the IbSystemControllerInteractive
           in the ``Integrationbus/bin/`` directory.


.. _sec:util-system-monitor:

IbSystemMonitor
~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemMonitor visualizes the states of the participants of an
         Integration Bus simulation.
   *  -  Source location
      -  ``Utilities/IbSystemMonitor``
   *  -  Requirements
      -  Requires a running IbRegistry to connect to.
   *  -  Parameters
      -  -v, --version                           Get version info.
         -h, --help                              Show the help of the IbSystemMonitor.
         -u, --connect-uri <vibUri>              The registry URI to connect to. Defaults to 'vib://localhost:8500'.
         -n, --name <participantName>            The participant name used to take part in the simulation. Defaults to 'SystemController'.
         -c, --configuration  <configuration>    Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.

   *  -  Usage Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor
            IbSystemMonitor
   *  -  Notes
      -  * The distribution package contains the IbSystemMonitor in the
           ``Integrationbus/bin/`` directory.
         * The SystemMonitor represents a passive participant in an Integration
           Bus system. Thus, it can be (re)started at any time.
