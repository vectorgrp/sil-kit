
Changelog
================================

All notable changes to the Vector SIL Kit project shall be documented in this file.

The format is based on `Keep a Changelog (http://keepachangelog.com/en/1.0.0/) <http://keepachangelog.com/en/1.0.0/>`_.

[unreleased]
---------------------


[4.0.52] - 2024-09-02 
---------------------

Fixed
~~~~~

- Fixed crash in ``sil-kit-registry`` utility that happened when the dashboard is enabled, but not actually available.

Added
~~~~~

- Message aggregation for simulations with time synchronization.
  Accessible via the experimental section in the Participant Configuration (Experimental | TimeSynchronization | EnableMessageAggregation).


[4.0.51] - 2024-07-18 
---------------------

Added
~~~~~

- Couple the virtual time to the wall clock. 
  An animation factor can be configured that describes how fast the simulation is allowed to run relative to the local wall clock.
  Accessible via a new experimental section in the Participant Configuration (Experimental | TimeSynchronization | AnimationFactor).
- Event flow documentation for the Network Simulation API.
- Registry (Dashboard): Automatically use bulk-endpoint if it is available
- Configuration option for structured logging in JSON format


[4.0.50] - 2024-05-15
---------------------

This is a Quality Assured Release.

Fixed
~~~~~

- Fixed crash in ``sil-kit-system-controller`` utility.
- Fixed source directory contents in ``.zip`` release archives.


[4.0.49] - 2024-05-08
---------------------

Changed
~~~~~~~

- SystemController utility: 
  Listens for OS signals to end simulation: Press ``[Ctrl]-[C]`` (SigInt) instead of ``[Enter]`` to end simulation.
  Better reports about the current system state in case of Error.

Fixed
~~~~~

- Valid state transition from ``Aborting`` to ``Shutdown`` no longer emits a warning message.

Added
~~~~~

- Participant Configuration: Support include semantics in participant configuration files/strings.
- Network Simulation: Experimental API for custom simulation of CAN, LIN, Ethernet and FlexRay networks.


[4.0.48] - 2024-04-15
---------------------

Changed
~~~~~~~

- We now use a linker script to limit the exported symbol visibility to the public C API and some legacy C++ symbols. 
- The ``--enable-dashboard`` CLI parameter for the registry is now a no-op. It is now activated when using ``--dashboard-uri`` or via the registry configuration. 

Fixed
~~~~~

- Fix MinGW build.
- Various fixes for dashboard integration.

Added
~~~~~

- Man pages for linux.
- All public struct members are now included by default in the documentation.


[4.0.47] - 2024-03-01
---------------------

Fixed
~~~~~

- Fixed building from the packaged sources (``SilKit-Source``).

- LIN Demo: Removed duplicate call to StartLifecyle when run as the LIN slave


[4.0.46] - 2024-02-27
---------------------

Fixed
~~~~~

- Added the ``*.manifest`` files to the source distribution. Building from the ``SilKit-Source``
  directory in the distributed ``.zip`` files was broken.

- Update ``yaml-cpp`` to version 0.8.0 to fix linker errors caused by a missing symbol.

- The system controller utility now logs parts of the command line output using the logger object
  of the participant.

Added
~~~~~

- Added a ``--log`` option to the system controller utility which cannot be used together with the
  ``--configuration`` option. It provides a shortcut to set the log level of the utility.

- Utilities: prepare the registry for handling multiple simulations

- Added a licensecheck to prevent source files without a license header

[4.0.45] - 2024-02-06
---------------------

Fixed
~~~~~

- Registry failed to start correctly, if the dashboard is enabled, but the registry is letting the
  system determine the listening port, e.g., when using a URI like ``silkit://localhost:0``.

Changed
~~~~~~~

- Add links to API sections in the documentation overview


Added
~~~~~

- RPC usage example with lifecycle

- Windows: Utilities and demos are now compiled with a manifest that sets the active codepage to UTF-8.
  The required commands to change the output codepage of the Windows console in ``cmd`` or PowerShell
  are documented in a new FAQ entry.


[4.0.44] - 2024-01-22
---------------------

Fixed
~~~~~

- Changing the TCP send and receive buffer size failed on Windows

Added
~~~~~

- Links to related SIL Kit projects in documentation and github Readme


[4.0.43] - 2023-12-12
---------------------

Fixed
~~~~~

- Dashboard can be enabled using CLI arguments
- Demos did not terminate when simulation is aborted (AbortSimulation)
- Fix order of debug log message parameters


[4.0.42] - 2023-11-29
---------------------

Changed
~~~~~~~

- LIN Demo: Adapted the schedule of the LIN Master and disallowed sending while in wrong controller state.


[4.0.41] - 2023-11-28
---------------------

Fixed
~~~~~

- Potential deadlock when switching to virtual time-synchronization while replay is in use


[4.0.40] - 2023-11-27
---------------------

Added
~~~~~

- Allow configuration of the connection timeout (``Middleware/ConnectTimeoutSeconds``)

Changed
~~~~~~~

- Improved the documentation of Data Pub/Sub controllers
- Improved the documentation of RPC controllers

Fixed
~~~~~

- The LIN demo does not skip the first entry (sending frame 16) on all but the first iteration through the schedule anymore.
- The name of the domain-socket used by the registry will use the hostname passed in the listen URI, not the resolved IP address (if any), for generating the name of the domain-socket.
- When mixing autonomous participants without time-synchronization, and participants with time-synchronization,
  the timestamps for messages received before the virtual time is started, is now the 'invalid' timestamp value,
  normally used by participants without time-synchronization.
- Reworked the pause/continue logic such that it pauses the virtual time synchronization without blocking the I/O thread.


[4.0.39] - 2023-11-14
---------------------

Fixed
~~~~~

- Replaced remaining mentions of integrators with users.

Added
~~~~~

- Usage examples for tracing and replay.
- Timeouts per connection attempt.
- Two-sided connection establishment: Allow direct connections, even if connections are only possible in one direction.


[4.0.38] - 2023-11-02
---------------------

Fixed
~~~~~

- The dashboard now handles AbortSimulation.
- The dashboard resolves the registry IP address if needed.


Added
~~~~~

- Reintroduced build requirements to documentation

Changed
~~~~~~~

- Consolidate SIL Kit tests into four executables


[4.0.37] - 2023-10-17
---------------------

Changed
~~~~~~~

- Reworked the documentation on Virtual Time Synchronization
- The documentation of the demo section now refers to the pre built Vector SIL Kit packages and not to a source build.

[4.0.36] - 2023-09-19
---------------------

Added
~~~~~

- Documentation on ``DashboardUri``


Fixed
~~~~~

- Fixed misbehavior of the sil-kit-system-controller in interactive mode on user input:

  - The sil-kit-system-controller now triggers a Stop() in SystemState::Running or SystemState::Paused.
  - The sil-kit-system-controller only triggers AbortSimulation when not SystemState::Running, SystemState::Paused, SystemState::Shutdown or SystemState::Aborting.

Changed
~~~~~~~

- Performance improvement of the internal serialization
- The final state handling of the sil-kit-system-controller in interactive mode on user input has changed:

  - Old: The sil-kit-system-controller triggered AbortSimulation if the finalState was not received after 5s. 
  - New: The sil-kit-system-controller retries receiving the finalState 3x5s. If this fails, the sil-kit-system-controller triggers AbortSimulation (if not already happened) and tries receiving the finalState 3x5s again. If this fails, the sil-kit-system-controller just terminates.

[4.0.35] - 2023-09-04
---------------------

Added
~~~~~

- Memory management documentation introduced.
- Integration tests for communication in the stop/shutdown/abort handlers.

Changed
~~~~~~~

- When building the SIL Kit documentation, the sphinx build command is no longer called in a pipenv. 

Fixed
~~~~~

- Ensured that calling ``ISystemController::AbortSimulation()`` does not lead to the system controller terminating
  prior to other participants receiving its abort message.
- Ensure that userContext field for external CanFrameTransmitEvents is always null.
- Fixed warning in VS2017 (x86) build  

[4.0.34] - 2023-08-21
---------------------

Changed
~~~~~~~

- Behavior change of ``ParticipantState::Error``

  - Old: Several situations could lead to an ``ParticipantState::Error`` before the user called ``StartLifecycle()``

    - Reception of an invalid ``WorkflowConfiguration``
    - Remote participant disconnected
    - Reception of ``AbortSimulation``

  - New: ``ParticipantState::Error`` should only be reached after ``StartLifecycle()`` was called

    - Reception of a WorkflowConfiguration is not validated before ``StartLifecycle()``
    - A disconnected remote participant is only transitioned to ``ParticipantState::Error`` if he had a started Lifecycle
    - Reaction on ``SystemCommang::AbortSimulation`` is deferred before ``StartLifecycle()`` (see below)

- Behavior change of ``SystemCommand::AbortSimulation``

  - Old: Reception of ``AbortSimulation`` before ``StartLifecycle()`` led to ``ParticipantState::Error``
  - New: Reception of ``AbortSimulation`` before ``StartLifecycle()`` is firstly ignored. A later call to ``StartLifecycle()`` then directly leads to an abort (transition to ``ParticipantState::Aborting``, calling the ``AbortHandler``)

- clang presets in ``CMakePresets.json`` now have the clang version in their names

- Added an internal barrier between ``ParticipantState::Shutdown`` and setting the final state promise. This ensures that the participant state updates are all transmitted while shutting down. 

- Revised log messages when shutting down / disconnecting participants

  - Graceful, participant has lifecycle: "Participant <participantName> has disconnected after gracefully shutting down",
  - Not graceful, participant has lifecycle: "Participant <participantName> has disconnected without gracefully shutting down."
  - Registry shutdown: "Connection to SIL Kit Registry was lost - no new participant connections can be established."
  - Participant without lifecycle: "Participant <participantName> has disconnected."


Fixed
~~~~~

- Made simulation time stop for all coordinated participants when one coordinated participant disconnects ungracefully


[4.0.33] - 2023-08-07
---------------------

Added
~~~~~

- New experimental extension of the LIN API that allows a user to send frame headers and respond to them
  without setting up a static configuration beforehand.
- Added link to FMU importer in Readme.md


Changed
~~~~~~~

- Restructured the documentation to have separate sections for overview.
  It is now divided into an overall introduction, a developer guide, and a user guide.
- Removed internal use of exceptions for failing connections to the registry if another connection mechanism succeeded. 


Fixed
~~~~~

- Fixed missing entry of RegistryAsFallbackProxy in YAML schema.


[4.0.32] - 2023-07-19
---------------------

This is a Quality Assured Release.

Fixed
~~~~~

- Ensure that the registry rejects a connecting participant if a participant with the same name
  is already connected.


[4.0.31] - 2023-07-10
---------------------

Added
~~~~~

- Improved Lifecycle and TimeSyncService features:

  - Full support for Operation Mode Autonomous with TimeSyncService including hopping onto / leaving a running simulation
  - Abort simulation in case Coordinated participants want to join a running simulation
  - Abort simulation in case an Autonomous with TimeSyncService sees an incompatible participant 
  - Extended integration tests for communication ready guarantees

Fixed
~~~~~

- Fixed a bug in internal barriers where in-between connecting participants could break the communicaiton guarantees
- Fixed transition when aborting from ErrorState, now the state changes to Shutdown like all aborting paths (formery the transition was to ShuttingDown)
- Internal fixes for thread-safety
- Fixed that the SimTask cannot be triggered again after calling ``ILifecycleService::Stop()`` in the SimTask


Changed
~~~~~~~

- CreateLifecycleService with OperationMode::Invalid now throws a ConfigurationError
- Improved documentation on how to run demos in asynchronous mode


[4.0.30] - 2023-06-26
---------------------

Added
~~~~~

- Added optional timeout mechanism to RPC service. 

Fixed
~~~~~

- Fix builds with CMake versions before ``3.19``
- Fixed crash in SilKitRpcDemo when run with ``<config> Client --async`` without a server.

Changed
~~~~~~~

- SIL Kit Demos (Can, Ethernet, Rpc, Lin, and PubSub) with ``--async`` now use an autonomous lifecycle.
- Refactored documentation for participant configurations: The intent was made clearer, noting that it is an optional feature.


[4.0.29] - 2023-06-14
---------------------

Fixed
~~~~~

- Registry: Set windows service state to ``stopped`` on error


[4.0.28] - 2023-06-02
---------------------

Added
~~~~~

- Added documentation for Data Serialization/Deserialization (SerDes) API

Fixed
~~~~~

- Fixed inconsistencies in API documentation
- Fixed starting the registry without the ``--registry-configuration`` parameter


[4.0.27] - 2023-05-30
---------------------

Changed
~~~~~~~

- Added detection of simulation start and end for the dashboard. 

  - The simulation id initialization is deferred until the first dashboard relevant event happens.
  - A simulation is considered as ended, when the last participant disconnects.

Added
~~~~~

- SIL Kit Registry (``sil-kit-registry(.exe)``)

  - Support for overriding command line settings via a YAML configuration file


[4.0.26] - 2023-05-22
---------------------

Added
~~~~~

- Introducing an internal communication barrier between participant states. This happens in the transition from the states ServicesCreated to CommunicationInitializing and from Stopped to ShuttingDown. It leads to extended communication guarantees in the CommunicationReadyHandler, primarily for participants that use the autonomous lifecycle (see the documentation for details).

Fixed
~~~~~

- Corrected spelling mistakes in the documentation
- The dashboard is disabled for cross-builds to QNX, to avoid build errors


[4.0.25] - 2023-05-17
---------------------

This version was skipped due to compatibility issues with dependent projects.


[4.0.24] - 2023-05-04
---------------------

Known issue: A LIN transmission by a LIN Slave may have a timestamp that is smaller than the LinSendFrameHeaderRequest of the LIN Master that triggered the Slave response. 

Added
~~~~~

- Header-only C++ API implementation following the hourglass-pattern

  - The C++ symbols are still provided by the shared library, but are not used by default anymore

- If a participant cannot establish a direct connection to another participant,
  it will fall back to using the registry as a proxy for communications with
  this particular participant

  - Support can be disabled on a particular participant using the new
    "Middleware/RegistryAsFallbackProxy" field in the participant configuration

Fixed
~~~~~

- Messages are sent in the same order as the SIL Kit API calls that triggered
  them, regardless of the thread or handler the API calls were executed from.
  There are no ordering guarantees for API calls that are executed in parallel
  from different threads.


[4.0.23] - 2023-04-17
---------------------

Changed
~~~~~~~

- Added network simulator information to the dashboard REST API

Fixed
~~~~~

- Fixed data type of simulation id returned by the dashboard REST API

- Ethernet and CAN (Trivial Sim.): The self-delivery with ``TransmitDirection::TX`` is now triggered only after
  the frame has been sent with ``TransmitDirection::RX``.


Removed
~~~~~~~

- Visual Studio 2015 is no longer maintained and therefore not officially supported anymore.


[4.0.22] - 2023-04-05
---------------------

Fixed
~~~~~

- Resolved issue that lead to wrong label matching behavior under certain circumstances.


[4.0.21] - 2023-04-03
---------------------

Changed
~~~~~~~

- Improved FAQ
- Preparation for upcoming tracing / replay 


[4.0.20] - 2023-03-20
---------------------

Changed
~~~~~~~

- Make additional data and rpc information available to the dashboard.

Added
~~~~~

- Added frequently asked questions (FAQ) section to documentation.

- Modification of BenchmarkDemo: Change the communication topology by modifying the PubSub topics. 
  A participant should only send to a single other participant.

- Add LatencyDemo: Measure the average latency between two participants in different processes.


Fixed
~~~~~

- C-API: Fixed a bug where the ``SilKit_EthernetFrameEvent`` delivered in the Ethernet frame handler had
  the ``userContext`` field always set to ``nullptr``, instead of the value passed in the corresponding ``SilKit_EthernetController_SendFrame`` call.
- C++-API: Fixed a bug where the ``userContext`` was set in the frame handlers registered on other controllers than the one calling ``SendFrame``. The ``userContext`` is only ever set when a frame event with ``TransmitDirection::TX`` is received, which is only possible on the same controller that sent it.


[4.0.19] - 2023-03-02
---------------------

Changed
~~~~~~~

- Use function-try-blocks for C-API definitions to reduce indentation and reformat some code
  for better readability.

Fixed
~~~~~

- Allow installing and starting the sil-kit-registry.exe as a Windows Service on Windows Containers during ``docker build`` steps.
  The layer creation fails if a domain socket is still active during shutdown of the temporary container.
  Disables the local-domain sockets of the ``sil-kit-registry.exe`` when running as a Windows Service.

- When a participant is unable to connect to another participant, the correct
  error message is logged, and an error is raised.


[4.0.18] - 2023-02-21
---------------------

Added
~~~~~

- Added descriptions to troubleshooting section for common errors.

Changed
~~~~~~~

- Complete the Hourglass implementation of the C++ API used internally for testing.

Fixed
~~~~~

- Properly handle IPv6 acceptors in the registry when transmitting to a remote participant


[4.0.17] - 2023-02-09
---------------------
This is a Quality Assured Release.

Fixed
~~~~~
- Fix DNS resolver issues on Ubuntu 18.04 and systemd-resolved. We now properly
  strip square brackets from IPv6 addresses in URIs.
- Fixed a race-condition in the ITest_SystemMonitor. This caused sporadic failures on CI builds.

Removed
~~~~~~~
- Removed the deprecated CMakeSettings.json file from the source tree.
  If you are a developer, use the CMakePreset.json instead which is more portable and flexible.
- Removed `usr/share/doc` from the delivery packages. This should only be part of Debian packages.



[4.0.16] - 2023-02-03
---------------------

Changed
~~~~~~~

- Improved error messages when connections between participants/to the registry have failed.

- CMake: Reduced weak symbols exported in debug builds.

- Allow configuration of acceptor URIs in the participant configuration.

- The third party dependencies were updated.

  - ``fmt`` to version 9.1.0.

  - ``spdlog`` to version 1.11.0.


[4.0.15] - 2023-01-23
---------------------

Changed
~~~~~~~

- Registry:

  - Allow running the ``sil-kit-registry`` as a windows service.
    In this case, the registry grants others the ``PROCESS_QUERY_LIMITED_INFORMATION`` permission.

  - The registry must be run with the ``--windows-service`` command line flag.
    This argument is *not* shown in the command line usage information available via ``--help``.
    This command line flag may be removed in the future.

- Docs: Add registry requirement to demos documentation.

- CMake: The minimum required CMake version has been bumped to 3.10


[4.0.14] - 2023-01-10
---------------------

Added
~~~~~

- CMake: Prepared cross-compiling for QNX

Changed
~~~~~~~

- Tests: Improved tests regarding configuration parsing

Fixed
~~~~~

- Catch and print exceptions when parsing utility CLI arguments
- Various issues found by the Address Sanitizer and Thread Sanitizer of Clang 14 are now resolved.


[4.0.13] - 2022-12-14
---------------------

Changed
~~~~~~~

- PubSub/Rpc: Improved performance in startup-phase when using labels.
- Documentation: Configuration structure for PCAP tracing and replay

Fixed
~~~~~

- LIN: Previously, only when using the network simulator, the FrameStatusHandler on the LIN Master was not called in
  case of an unconfigured response (RX_NO_RESPONSE). This inconsistency has been fixed.

- Registry

  - The acceptor URIs the known participants list sent by the registry are now rewritten correctly.
    Previously, certain startup scenarios only worked when the participants were started in a certain order.

- SerDes: Added missing limit library include.


[4.0.12] - 2022-11-24
---------------------

Fixed
~~~~~

- Integration Tests

  - Linking executables with both, the dynamic library, and certain internal, static library components leads to ODR violations.
    The ASAN / UBSAN instrumentation from recent Clang versions is able to detect these.

- C-API Tests

  - Fixed some memory leaks in C-API tests.

- LIN

  - Fixed faulty behavior of ``ILinController::SendFrame()`` for ``LinFrameResponseType::MasterResponse``. The method now consistently uses the input frame data.

- Lifecycle Service

  - Alleviate potential loss of the 'stop' signal issued from the lifecycle service of a participant.

- Various fixes related to warnings

  - Remove duplicate variables in PubSub demo.
  
  - Fix 'D9025: overriding...' diagnostics in MSVC.
  
  - Fix warnings in hourglass code.
  
  - Fix cmake configuration on macos.
  
  - Properly initialize variable in unit test.

[4.0.11] - 2022-11-18
---------------------

Changed
~~~~~~~

- Ethernet

  - The Ethernet controller now quietly pads Ethernet frames with zeros to the
    minimum size of 60 bytes.

  - (Re-)added experimental support for PCAP tracing and replay on ethernet controllers.
    The tracing and replay behavior may change in the future.

- Registry

  - Only exit if neither TCP, nor domain sockets are available.

- SIL Kit Library

  - Changed the default symbol visibility to hidden, which is now also enforced for
    Linux builds.

  - Symbols for ASIO are not exported as weak symbols anymore.

Fixed
~~~~~

- C: CAN:

  - Added missing ``canId`` field to the ``SilKit_CanFrameTransmitEvent`` and bumped the structure version.


[4.0.10] - 2022-11-07
---------------------

Changed
~~~~~~~

- Documentation

  - Improved Quickstart Docs and fixed sample code
  - Updated build instructions for documentation

- Logging

  - Trace-Log was extended with outputs for controller creation.
  - Default verbosity of lifecycle reduced to debug
  
- Demos: Replaced JSON configuration files with YAML files. The provided configurations did not change.
- Extended SilKitDemoBenchmark

    - Calculates standard deviation for throughput, message rate, speedup
    - Added ``--write-csv`` command line argument to output results to csv file
    - Helper scripts for msg-size-scaling and performance-diff

- Changed the lifecycle service to be less verbose in log level info. See log level debug for more detailed
  information of the lifecycle.

- Updated participant configuration file schema and added it to the json schema support. Use .silkit.yaml/json suffix 
  for automated schema support.


[4.0.9] - 2022-10-19
--------------------

Added
~~~~~

- The LinDemo, PubSubDemo, and RpcDemo now allow using ``--async`` cli flag for unsynchronized execution.

Fixed
~~~~~

- C-API:

  - Data race on static variable

  - Setting the direction field of the ``CanFrameEvent``

  - Initialize the struct header of the embedded ``CanFrame`` structure in the ``CanFrameEvent``

- Immediate shutdown of the asynchronous mode of the LIN demo


[4.0.8] - 2022-10-07
--------------------

Changed
~~~~~~~

- The third party dependencies were updated.

  - ``yaml-cpp`` to version 0.7.0.

  - ``asio`` to version 1.24.0.

  - ``fmt`` to version 8.1.1.

  - ``spdlog`` to version 1.10.0.

  - ``googletest`` to version 1.12.1.

- The FlexRay controller now issues a warning if the static buffer payload is truncated
  or padded with zeros, i.e., if the size is not exactly as specified in the controller
  configuration.

- The registry now transmits a diagnostic string when a participant announcement cannot
  be processed, e.g., because a participant with the same name already exists.
  The second participant will not time out after a few seconds anymore, but fail much faster.

[4.0.7] - 2022-09-20
--------------------

Changed
~~~~~~~
- The documentation is now packaged separately in a `SilKit-4.0.7-Docs.zip` file.
  This simplifies the CI set up and reproducibility of the generated HTML.
  To build the documentation you should set up the build environment using pip:

  .. code-block:: sh

     pip3 install -r SilKit/ci/docker/docs_requirements.txt

- The source tree is now packaged separately in a `SilKit-4.0.7-Source.zip` file.
  The SIL Kit Demos are part of this package.

- We no longer have a `#if defined(HAVE_FMT)` in the `silkit/services/logging/ILogger.hpp`
  and variadic logging functions.
  This define was disabled by default for users and only used internally.

- LIN allows sending with an unknown checksum model in master responses, now.

- The Pub/Sub Demo was updated to use the internal serialization/deserialization routines.

Fixed
~~~~~
- Fix building and linking on macOs. This platform is not part of the continuous test suite.
- Fix to allow setting the hard and soft watchdog timeouts in the HealthCheck separately.

[4.0.6] - 2022-09-06
--------------------

Changed
~~~~~~~

- Removed an empty directory from the packages

- Updated the description of SIL Kit in the top-level README.rst

Fixed
~~~~~

- Clean up peers after a remote participant disconnects

- Windows: Fixed the internal name and original filename attributes of the ``sil-kit-monitor.exe``


[4.0.5] - 2022-08-25 Initial public release (quality assured release)
---------------------------------------------------------------------

This is the first public open source release of the Vector SIL Kit.

Starting with this version, Vector SIL Kit will provide longterm API, ABI, and network compatibility. Note that prior versions do not provide this compatibility.


[4.0.4] - 2022-08-22
--------------------

Compatibility with 4.0.3
~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): No
- Middleware network protocol: No

Changed
~~~~~~~

- Utility (SerDes)

  - ``SilKit/include/silkit/util/serdes/Serialization.hpp``:

    - The media type for PubSub was changed from ``application/vnd.vector.sil.data; protocolVersion=1`` to ``application/vnd.vector.silkit.data; protocolVersion=1``

    - The media type for RPC was changed from ``application/vnd.vector.sil.rpc; protocolVersion=1`` to ``application/vnd.vector.silkit.rpc; protocolVersion=1``

- LIN

  - ``SilKit_LinChecksumModel_Undefined`` was renamed to ``SilKit_LinChecksumModel_Unknown``.

- C: Orchestration

  - ``SilKit_LifecycleService_Stop`` was added to the C-API.
    This corresponds to ``SilKit::Services::Orchestration::ILifecycleService::Stop``.

- C: Ethernet

  - Frames delivered in user-provided ``SilKit_EthernetFrameHandler`` functions had an invalid payload delivered.
    This was fixed, the frame is now correctly delivered.
    The error only occured in the C API, the C++ API correctly delivered the entire frame.


[4.0.3] - 2022-08-22
--------------------

Compatibility with 4.0.2
~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: Yes

Added
~~~~~

- LIN

  - ``SilKit/include/silkit/services/lin/ILinController.hpp``:

    - The new method `SetFrameResponse` allows LIN nodes to adjust their LIN response configuration during operation
      (i.e., after Init()). Calls to `SetFrameResponse` trigger the `LinSlaveConfigurationHandler` on the LIN master.

    - The LIN types `LinChecksumModel::Unknown` and / or `LinDataLengthUnknown` now have wildcard functionality for
      nodes  configured with `LinFrameResponseMode::RX` on that id. The first transmission will set the checksum model
      and / or data length.

- CAN

  - Added ``SilKit_CanTransmitStatus_DefaultMask``.

- Ethernet

  - Added ``SilKit_EthernetTransmitStatus_DefaultMask``.

Changed
~~~~~~~

- LIN

  - ``SilKit/include/silkit/services/lin/ILinController.hpp``:

    - The methods `AddLinSlaveConfigurationHandler`, `RemoveLinSlaveConfigurationHandler`, `GetSlaveConfiguration` and 
      related data structures `LinSlaveConfigurationEvent`, `LinSlaveConfigurationHandler` and `LinSlaveConfiguration`
      have been moved to the experimental namespace and now reside in  
      ``SilKit/include/silkit/experimental/services/lin/LinControllerExtensions.hpp``.
    - `LinChecksumModel::Undefined` is renamed to `LinChecksumModel::Unknown`.

- Utility

  - Moved the headers from ``silkit/util/serdes/sil/*.hpp`` to ``silkit/util/serdes/*.hpp``.

- CAN

  - Removed unused ``SilKit_CanTransmitStatus_DuplicatedTransmitId`` and ``SilKit::Services::Can::CanTransmitStatus::DuplicatedTransmitId`` enumerators.

- Ethernet

  - Removed unused ``SilKit_EthernetTransmitStatus_DuplicatedTransmitId`` and ``SilKit::Services::Ethernet::EthernetTransmitStatus::DuplicatedTransmitId`` enumerators.

- FlexRay

  - Renamed ``FlexrayHeader::HeaderFlag`` to ``FlexrayHeader::Flag`` and introduced ``FlexrayHeader::FlagMask``.

- RPC

  - Renamed ``RpcSpec::Topic`` to ``RpcSpec::FunctionName``.
  - Renamed enumerators ``SilKit_CallStatus_UPPER_SNAKE_CASE`` to ``SilKit_RpcCallStatus_PascalCase``.

Removed
~~~~~~~

- Orchestration

  - Removed the deprecated ``ITimeSyncService::SetSimulationStepHandler`` handler which took a handler function without the ``duration`` argument.

- FlexRay

  - Removed the convenience functions ``FlexrayHeader::IsSet``, ``FlexrayHeader::Clear``, and ``FlexrayHeader::Set``.

Fixed
~~~~~

- Coordinated, but non-required participants that received the required participant list before calling 
  `ILifecycleService::StartLifecycle()` did not go to the error state.


[4.0.2] - 2022-08-15
--------------------

Compatibility with 4.0.1
~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol: Yes

Changed
~~~~~~~

- SilKit will now build on NetBSD (currently not tested automatically)

[4.0.1] - 2022-08-15
--------------------

Compatibility with 4.0.0
~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol: Yes

Changed
~~~~~~~

- The calling convention used in the C-API was changed to ``cdecl`` on 32-bit Windows.

Removed
~~~~~~~

- The ``sil-kit-registry`` command line argument ``--configuration`` was removed.


[4.0.0] - 2022-08-11
--------------------

Compatibility with 3.99.30
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: No

Added
~~~~~

- Added the ``SILKIT_ENABLE_COVERAGE`` CMake option for adding code-coverage compiler flags when building with GCC or Clang.

Changed
~~~~~~~

- Added an CLI argument to sil-kit-system-controller to run it without user interaction (--non-interactive or -ni)
- Participants to not go to `ParticipantState::Error` anymore when the system state changes to `SystemState::Error`

Removed
~~~~~~~

- Removed various demos (CCan, CEthernet, CFlexray, CLin, Lifecycle, and TimeAnnotation) as they were meant for testing
- Removed benchmark demo

Fixed
~~~~~

- The sil-kit-system-controller crashed when any button was pressed. This is fixed.

[3.99.30] - 2022-08-09
----------------------

Compatibility with 3.99.29
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: No

Changed
~~~~~~~

- Using the same name controller name (`canonicalName` parameter in Create*Controller) in different networks 
  (`network` parameter in Create*Controller) is no longer allowed. Controller creation now requires 
  unique controller names within the same controller type. Any violation of this rule causes a `ConfigurationError`.

- PubSub/Rpc: Both sides (clients and servers, publishers and subscribers) now can specify the matching behavior of 
  individual labels. This is done via the `kind` field of a `MatchingLabel` which has to be specified when using 
  `AddLabel`. The `Label` struct has been is removed. Additionally, the 
  `SilKit::Services::MatchingLabel::Kind::Preferred` has been renamed to 
  `SilKit::Services::MatchingLabel::Kind::Optional`.

- RPC

  - Added new ``RpcCallStatus::InternalServerError`` and ``SilKit_CallStatus_INTERNAL_SERVER_ERROR``.
  - ``IRpcClient::Call`` now takes an additional ``userContext`` parameter and does not return a ``IRpcCallHandle *`` anymore.
    The ``userContext`` is presented in the ``RpcCallReturnHandler`` in the ``RpcCallReturnEvent`` structure instead of the ``callHandle``.

- Remove the unused and outdated `synchronized` parameter from `SilKit_Participant_Create`.

- Add and use opaque `SilKit_ParticipantConfiguration` type for use in `SilKit_Participant_Create`.

- C: Added ``SilKitCALL`` and ``SilKitFPTR`` macros for specification of the calling convention when building for windows.

- Participants cannot be configured to be coordinated if they are not required as well. In case of this combination, an exception is thrown.

- ``IParticipant.hpp``
  Functionality to aquire a controller by calling Create* twice is removed.   All methods for controller creation
  (Bus systems, PubSub, Rpc) now no longer return the cached controller pointer if called with same name and network,
  but throw a ConfiguraionError.

- The suffix 'T' has been removed in all handler identifiers (mainly 'using'-statements, e.g. 'DataMessageHandler' -> 'DataMessageHandlerT').

- Renamed file ``SilKit/include/silkit/services/orchestration/SyncDatatypes.hpp`` to ``OrchestrationDatatypes.hpp``.

- Participants may not be coordinated and not part of the required participants list

  - Currently, this will lead to an exception

- Lifecycle service changes

  - Instead of booleans, the ``Service::Orchestration::LifecycleConfiguration`` now comprises a single enumerator ``OperationMode`` that defines if a participant coordinates its state transition with others or if it runs autonomously.
  - Most SystemCommands and all ParticipantCommands were removed.
  - Participants will not wait for a commands to Initialize, Run, Stop, or Shutdown anymore. Instead, coordinated participants will react to system state changes.
  - Instead of calling `ISystemController::Stop()`, any required participant can stop all coordinated participants by calling `ILifecycleService::Stop()`.
  - Autonomous participants must call `ILifeCycleService::Stop()` by themselves.
  - All participants that arrive at the ``Stopped`` state now continue to ``Shutdown`` (via ``ShuttingDown``)
  - The ``Service::Orchestration::LifecycleConfiguration`` must now be provided in `IParticipant::CreateLifecycleService()` instead of `ILifecycleService::StartLifecycle()`

Added
~~~~~

- The C API now has methods to aquire SIL Kit version information in ``SilKit/include/silkit/capi/Version.h``:

  .. code-block:: c++

    SilKitAPI SilKit_ReturnCode SilKit_Version_Major(uint32_t* outVersionMajor);
    SilKitAPI SilKit_ReturnCode SilKit_Version_Minor(uint32_t* outVersionMinor);
    SilKitAPI SilKit_ReturnCode SilKit_Version_Patch(uint32_t* outVersionPatch);
    SilKitAPI SilKit_ReturnCode SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber);
    SilKitAPI SilKit_ReturnCode SilKit_Version_String(const char** outVersionString);
    SilKitAPI SilKit_ReturnCode SilKit_Version_VersionSuffix(const char** outVersionVersionSuffix);
    SilKitAPI SilKit_ReturnCode SilKit_Version_GitHash(const char** outVersionGitHash);

Removed
~~~~~~~

- ``SilKit/include/silkit/services/ethernet/EthernetDatatypes.hpp``:
  Removed field ``MacAdress`` from ``EthernetFrameTransmitEvent``.


[3.99.29] - 28-07-2022
----------------------

Compatibility with 3.99.28
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: Yes

Added
~~~~~

- Added a :cpp:func:`SetCommunicationReadyHandlerAsync<SilKit::Services::Orchestration::ILifecycleServiceNoTime::SetCommunicationReadyHandlerAsync>`
  method to the lifecycle interfaces.
  It will invoke the :cpp:type:`CommunicationReadyHandler<SilKit::Services::Orchestration::CommunicationReadyHandler>` callback in a separate thread.
  This allows the user to do early communication in a simulation run, for example, to
  exchange configuration values before the actual simulation starts.
  The user is required to call :cpp:func:`CompleteCommunicationReadyHandlerAsync<SilKit::Services::Orchestration::ILifecycleServiceNoTime::CompleteCommunicationReadyHandlerAsync>` when the handler is finished.

Changed
~~~~~~~

- Changed access to Logger so that it can be obtained at every time

  - ``IntegrationBus/include/silkit/participant/IParticipant.hpp``

    + old:

      .. code-block:: c++

        virtual auto CreateLogger() -> Services::Logging::ILogger* = 0;

    + new:

      .. code-block:: c++

        virtual auto GetLogger() -> Services::Logging::ILogger* = 0;

- ``sil-kit-registry`` now has an addition argument ``--generate-configuration`` which can be used in CI environments
  together with a OS generated port in the URI (i.e. ``silkit://localhost:0``) to create a basic configuration file
  containing the actual port on which the registry is reachable.


[3.99.28] - 2022-07-26
----------------------

Compatibility with 3.99.27
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: No

Changed
~~~~~~~

- LIN: API Overhaul

  - Changed behavior of ``ILinController::SendFrame()`` and ``ILinController::SendFrameHeader()``:
    Both now don't use cached responsed but send the LinHeader to the responding LIN node and use the TxBuffer there.

  - The method ``ILinController::SetFrameResponse()`` and ``ILinController::SetFrameResponses()`` have been removed.
    LIN controllers now have to hand in their final reponse configuration (Tx/Rx) in ``ILinController::Init()`` and
    can't reconfigure their configuration afterwards. An exception is the LIN master when using 
    ``ILinController::SendFrame()`` with ``LinFrameResponseType::MasterResponse`` or 
    ``LinFrameResponseType::SlaveResponse``, which reconfigures the LIN master during operation.
  - The new method ``ILinController::UpdateTxBuffer()`` can be used to update the payload for a certain LIN ID,
    but does not change the response configuration.
  - The ``FrameResponseUpdateHandler`` has been removed. An alternative way of obtaining knowledge about response
    configuration of slaves on the master is the ``LinSlaveConfigurationHandler``. This handler triggers when a 
    LIN slave calls ``ILinController::Init()``. Inside the handler, the new method 
    ``ILinController::GetSlaveConfiguration()`` can be used to query on which LIN IDs any slave is configure for 
    response. This allows to implement a bookkeeping mechanism on the master and predict if a slave response is 
    expected.

- Renamed SimulationTask to SimulationStep and added the initial step size (formerly period length) as a parameter

  - ``IntegrationBus/include/silkit/services/orchestration/ITimeSyncService.hpp``

    + old:

      .. code-block:: c++

        virtual void SetSimulationTask(SimTaskT task) = 0;
        virtual void SetSimulationTaskAsync(SimTaskT task) = 0;

    + new:

      .. code-block:: c++

        virtual void SetSimulationStepHandler(SimTaskT task, std::chrono::nanoseconds initialStepSize) = 0;
        virtual void SetSimulationStepHandlerAsync(SimTaskT task, std::chrono::nanoseconds initialStepSize) = 0;

- Changed access to services that are meant to exist only once (SystemController, SystemMonitor, Logger, LifecycleService)

  - Methods to access these services were renamed from ``Get[Service]()`` to ``Create[Service]()``
  - ``IntegrationBus/include/silkit/participant/IParticipant.hpp``

    + old:

      .. code-block:: c++

        virtual auto GetLifecycleService() -> Services::Orchestration::ILifecycleService* = 0;
        virtual auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* = 0;
        virtual auto GetSystemController() -> Services::Orchestration::ISystemController* = 0;
        virtual auto GetLogger() -> Services::Logging::ILogger* = 0;

    + new:

      .. code-block:: c++

        virtual auto CreateLifecycleService() -> Services::Orchestration::ILifecycleService* = 0;
        virtual auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* = 0;
        virtual auto CreateSystemController() -> Services::Orchestration::ISystemController* = 0;
        virtual auto CreateLogger() -> Services::Logging::ILogger* = 0;

  - The changed methods can only be called once per participant. Further calls throw a runtime_error.

- Instead of setting the time synchronization behavior when starting the lifecycle (``ILifecycleService::StartLifecycleNoTimeSync`` or ``ILifecycleService::StartLifecycleWithTimeSync``), the synchronization behavior is now determined when creating the lifecycle service

  - ``IntegrationBus/include/silkit/participant/IParticipant.hpp``

    + old:

      .. code-block:: c++

        virtual auto CreateLifecycleService() -> Services::Orchestration::ILifecycleService* = 0;

    + new:

      .. code-block:: c++

        virtual auto CreateLifecycleServiceNoTimeSync() -> Services::Orchestration::ILifecycleServiceNoTimeSync* = 0;
        virtual auto CreateLifecycleServiceWithTimeSync() -> Services::Orchestration::ILifecycleServiceWithTimeSync* = 0;

  - ``IntegrationBus/include/silkit/services/orchestration/ILifecycleService.hpp``

    + old:

      .. code-block:: c++

        virtual auto StartLifecycleNoSyncTime(LifecycleConfiguration startConfiguration) -> std::future<ParticipantState> = 0;
        virtual auto StartLifecycleWithSyncTime(LifecycleConfiguration startConfiguration ) -> std::future<ParticipantState> = 0;


    + new:

      .. code-block:: c++

        virtual auto StartLifecycle(LifecycleConfiguration startConfiguration ) -> std::future<ParticipantState> = 0;

  - The new create method returns interfaces that only comprises available methods
    -  ``ILifecycleServiceNoTimeSync::SetStartingHandler()`` without time synchronization
    -  ``ILifecycleServiceWithTimeSync::GetTimeSyncService()`` with time synchronization

- C\+\+: Extended the ``CanFrame`` with the required fields for CAN XL.
  The flags bitfield was replaced with an unsigned integer field (``uint32_t``) and a ``CanFrameFlag`` enumeration.

- C\+\+: Extended the ``ICanController::SetBaudRate`` function with the CAN XL data bit rate.

- C: Extended the ``SilKit_CanFrame`` with the required fields for CAN XL.

- C: Extended the ``SilKit_CanController_SetBaudRate`` function with the CAN XL data bit rate.

- Changed RPC label matching
  
  - ``IntegrationBus/include/silkit/participant/Iparticipant.hpp``
  
    + old:
  
      .. code-block:: c++

        virtual auto CreateRpcClient(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient* = 0;
        virtual auto CreateRpcServer(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer* = 0;

    + new:
  
      .. code-block:: c++

        virtual auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcClientSpec& dataSpec,
                                 Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient* = 0;
        virtual auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcServerSpec& dataSpec,
                                 Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer* = 0;

- Changed Data Publish Subscribe label matching
  
  - ``IntegrationBus/include/silkit/participant/Iparticipant.hpp``
  
    + old:
  
      .. code-block:: c++

        virtual auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
                                     const std::string& mediaType,
                                     const std::map<std::string, std::string>& labels, size_t history = 0)
        virtual auto CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                                      const std::string& mediaType,
                                      const std::map<std::string, std::string>& labels,
                                      Services::PubSub::DataMessageHandlerT defaultDataMessageHandler,
                                      Services::PubSub::NewDataPublisherHandlerT newDataPublisherHandler = nullptr)

    + new:
  
      .. code-block:: c++

        virtual auto CreateDataPublisher(const std::string& canonicalName, SilKit::Services::PubSub::DataPublisherSpec& dataSpec, size_t history = 0)
        virtual auto CreateDataSubscriber(const std::string& canonicalName, SilKit::Services::PubSub::DataSubscriberSpec& dataSpec,
                                      Services::PubSub::DataMessageHandlerT dataMessageHandler)

  - ``IntegrationBus/include/silkit/services/pubsub/IDataSubscriber.hpp``
  
    + old:
  
      .. code-block:: c++

        virtual void SetDefaultDataMessageHandler(DataMessageHandlerT callback) = 0;

    + new:
  
      .. code-block:: c++

        virtual void SetDataMessageHandler(DataMessageHandlerT callback) = 0;

- C: Added the simulation step duration to the ``SilKit_TimeSyncService_SimulationStepHandler_t`` callback

- C\+\+: Extended the ``IEthernetController::AddFrameHandler`` function with the ``directionMask`` filter, similar to ``ICanController::AddFrameHandler``.

- C\+\+: Extended the ``IEthernetController::AddFrameTransmitHandler`` function with the ``transmitStatusMask`` filter, similar to ``ICanController::AddFrameTransmitHandler``.

- C\+\+: Extended the ``IEthernetController::SendFrame`` function with the ``userContext`` argument, similar to ``ICanController::SendFrame``.

- C\+\+: The ``EthernetTransmitStatus`` enumerators are now individual bits and can be used in the ``transmitStatusMask`` argument.

- C: Extended ``SilKit_EthernetController_AddFrameHandler`` with the ``directionMask`` filter.

- C: Extended ``SilKit_EthernetController_AddFrameTransmitHandler`` with the ``transmitStatusMask`` filter.

Removed
~~~~~~~

- Removed ``ITimeSyncService::SetPeriod()`` (now provided via ``ITimeSyncService::SetSimulationStepHandler()``)

- Removed RPC Discovery functionalities

- Removed specific data handler functionality

- Removed functional.hpp utility header


[3.99.27] - 2022-07-14
----------------------

Please note that the Vector IntegrationBus was renamed to Vector SIL Kit.
All APIs and documentation have been updated to reflect this.

Compatibility with 3.99.26
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol: No

Added
~~~~~~~

- Logger now provides an API to get the currently active log level.

  + ``SilKit/include/services/logging/ILogger.h``

    .. code-block:: c++

      virtual Level GetLogLevel() const = 0;


Changed
~~~~~~~

- Renaming the IntegrationBus to SIL Kit affects all APIs.

    - In general, **File names** and **symbols** were renamed from the prefixes ``Ib``
      and ``IntegrationBus`` to the prefix ``SilKit``.

    - The main source directory was renamed from ``IntegrationBus`` to ``SilKit``
      and the include directories are now consistently in lower-case and with a
      root directory of ``silkit``.

    - Packages are now named ``SilKit-X.Y.Z-tool-platform.zip``.

    - C++ namespaces were renamed:

      .. list-table:: : C++ namespace changes
         :widths: 40 40
         :header-rows: 1
      
         * - Old
           - New
         * - ``ib::``
           - ``SilKit::``
         * - ``ib::mw``
           - not public anymore
         * - ``ib::sim``
           - ``SilKit::Services``
         * - ``ib::sim::eth``
           - ``SilKit::Services::Ethernet``
         * - ``ib::sim::can``
           - ``SilKit::Services::Can``
         * - ``ib::sim::lin``
           - ``SilKit::Services::Lin``
         * - ``ib::sim::fr``
           - ``SilKit::Services::Flexray``
         * - ``ib::sim::data``
           - ``SilKit::Services::PubSub``
         * - ``ib::sim::rpc``
           - ``SilKit::Services::Rpc``
         * - ``ib::mw::sync``
           - ``SilKit::Services::Orchestration``
         * - ``ib::mw::logging``
           - ``SilKit::Services::Logging``
         * - ``ib::cfg``
           - ``SilKit::Config``

- C++ general cleanup:
    - renamed ``ib/version.hpp`` to ``silkit/SilKitVersion.hpp``
    - moved ``ib/IParticipant.hpp`` and ``ib/exception.hpp`` to ``silkit/participant/``

- C-API: improvements for longterm ABI stability.
  The ``interfaceId`` member was replaced with a more versatile structHeader of type SilKit_StructHeader.
  This is a private field and not ment to be changed by the user directly.
  It is now necessary to initialize data structures before passing them to the C-API using the `SilKit_Struct_Init` macro.
  For example:

    + old:
  
      .. code-block:: c
 
        SilKit_CanFrame canFrame;
        /* we could pass uninitialized data to SIL Kit */
        SilKit_CanController_SendFrame(canController, &canFrame, NULL);
    + new:
  
      .. code-block:: c

        SilKit_CanFrame canFrame;
        /* we must initialize the data structures header before use */
        SilKit_Struct_Init(SilKit_CanFrame, canFrame);
        SilKit_CanController_SendFrame(canController, &canFrame, NULL);
 
- C-API: the C symbols have been stream lined. The naming convention was changed from 
  ``ib_Namespace_EntityWithoutPrefix_Function`` to resemble the C++ API:
  ``SilKit_Entity_Function``.

- The domain ID integer was removed and replaced with a registry URI string.
  The command line tools were updated to accept a new parameter for this.
- The command line tools were modified to use lower case names with dashes:
  E.g., the ``IbRegistry`` is now called ``sil-kit-registry``.
  See  :doc:`./usage/utilities`  for details.

- The trivial simulation and the detailed simulation have been made more consistent:

  + ``ILinController::SendFrame``, ``ILinController::SendFrameHeader``, and ``ILinController::SetResponses`` now throw an ib::StateError if the controller has not been initialized

  + ``IEthernetController::SendFrame`` now triggers a TransmitFrameEvent with TransmitState::ControllerInactive if the controller has not been activated

  + ``ICanController::SendFrame`` does not send a frame, but prints a warning if the controller has not been started

- The timestamps for received events is now dependent on the synchronization mode of the sender and the receiver

  .. list-table:: : Message timestamp by synchronization mode
     :widths: 20 40 40
     :header-rows: 1
  
     * - Sender / Receiver
       - Unsynchronized
       - Synchronized
     * - Unsynchronized
       - Undefined
       - Use timestamp of own simulation step
     * - Synchronized
       - Undefined
       - Use timestamp of sender

- The orchestration services were restructured in the  C API such that they are more consistent with the Cpp API.
  The API of the system controller, system monitor, lifecycle service, and the time sync service are now provided through
  SilKit_SystemController, SilKit_SystemMonitor, SilKit_LifecycleService, and SilKit_TimeSyncService:

  + ``SilKit/include/capi/Orchestration.h``

    .. code-block:: c++

      SilKit_ReturnCode SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                        SilKit_Participant* participant);
      SilKit_ReturnCode SilKit_SystemController_Create(SilKit_SystemController** outSystemController,
                                                        SilKit_Participant* participant);
      SilKit_ReturnCode SilKit_SystemController_Create(SilKit_SystemController** outSystemController,
                                                        SilKit_Participant* participant);
      SilKit_ReturnCode SilKit_LifecycleService_Create(SilKit_LifecycleService** outLifecycleService,
                                                           SilKit_Participant* participant);
      SilKit_ReturnCode SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService,
                                                               SilKit_LifecycleService* lifecycleService);
      typedef void (*SilKit_LifecycleService_CommunicationReadyHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

      SilKit_ReturnCode SilKit_LifecycleService_SetCommunicationReadyHandler(
             SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);
      SilKit_ReturnCode SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* lifecycleService, void* context,
                                                              SilKit_LifecycleService_StopHandler_t handler);
      SilKit_ReturnCode SilKit_LifecycleService_SetShutdownHandler(
                 SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler);
      SilKit_ReturnCode SilKit_TimeSyncService_SetPeriod(SilKit_TimeSyncService* timeSyncService,
                                                         SilKit_NanosecondsTime period);
      typedef void (*SilKit_TimeSyncService_SimulationTaskHandler_t)(void* context, SilKit_TimeSyncService* timeSyncService,
                                                          SilKit_NanosecondsTime now);
      SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationTask(
              SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);
      SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationTaskAsync(
              SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);
      SilKit_ReturnCode SilKit_TimeSyncService_CompleteSimulationTask(SilKit_TimeSyncService* timeSyncService);
      SilKit_ReturnCode SilKit_SystemController_Restart(SilKit_SystemController* systemController, const char* participantName);
      SilKit_ReturnCode SilKit_SystemController_Run(SilKit_SystemController* systemController);
      SilKit_ReturnCode SilKit_SystemController_Stop(SilKit_SystemController* systemController);
      SilKit_ReturnCode SilKit_SystemController_Shutdown(SilKit_SystemController* systemController,
                                                             const char* participantName);
      SilKit_ReturnCode SilKit_LifecycleService_Pause(SilKit_LifecycleService* lifecycleService, const char* reason);
      SilKit_ReturnCode SilKit_LifecycleService_Continue(SilKit_LifecycleService* lifecycleService);
      SilKit_ReturnCode SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                                   SilKit_Participant* participant,
                                                                   const char* participantName);
      SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outSystemState,
                                                              SilKit_Participant* participant);
      SilKit_ReturnCode SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                       void* context,
                                                                       SilKit_SystemStateHandler_t handler,
                                                                       SilKit_HandlerId* outHandlerId);
      SilKit_ReturnCode SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                          SilKit_HandlerId handlerId);
      typedef void (*SilKit_ParticipantStatusHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
                                                  const char* participantName, SilKit_ParticipantStatus* status);
      SilKit_ReturnCode SilKit_SystemMonitor_AddParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                             void* context,
                                                                             SilKit_ParticipantStatusHandler_t handler,
                                                                             SilKit_HandlerId* outHandlerId);
      SilKit_ReturnCode SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                                SilKit_HandlerId handlerId);
      SilKit_ReturnCode SilKit_SystemController_SetWorkflowConfiguration(
                 SilKit_SystemController* systemController, const SilKit_WorkflowConfiguration* workflowConfigration);
      SilKit_ReturnCode SilKit_LifecycleService_StartLifecycleNoSyncTime(
                        SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startconfiguration);
      SilKit_LifecycleService_StartLifecycleWithSyncTime(
                        SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startConfiguration);
      SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_WaitForLifecycleToComplete(
                             SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);

- The callbacks of ``ISystemMonitor::OnParticipantConnected`` and ``ISystemMonitor::OnParticipantDisConnected`` now return a struct that contains the information about the (dis)connected participant instead of a string.
  
  + Currently, the only information in this struct is the name of the participant 

Removed
~~~~~~~

- The documentation of the network simulator has been moved to its own repository.

- The documentation of the tracing and replay features were removed.

- Removed simple Create...Controller API for a more compact API

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    .. code-block:: c++

      virtual auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* = 0;
      virtual auto CreateEthernetController(const std::string& canonicalName) -> sim::eth::IEthernetController* = 0;
      virtual auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFlexrayController* = 0;
      virtual auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* = 0;
      virtual auto CreateDataPublisher(const std::string& canonicalName) -> sim::data::IDataPublisher* = 0;
      virtual auto CreateDataSubscriber(const std::string& canonicalName) -> sim::data::IDataSubscriber* = 0;
      virtual auto CreateRpcClient(const std::string& canonicalName) -> sim::rpc::IRpcClient* = 0;
      virtual auto CreateRpcServer(const std::string& canonicalName) -> sim::rpc::IRpcServer* = 0;

Fixed
~~~~~
- Ensure that the SynchronizedPolicy object does not modify the Timeconfiguration.
  This prevents multiple invocations of an async SimTask (VIB-847).

[3.99.26] - 2022-06-29
----------------------

Compatibility with 3.99.25
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): No

Added
~~~~~
- IbSystemControllerInteractive: Added ``Abort`` as possible input

- the new ILifeCycleService is now exposed on the C-API:
  added the new :cpp:func:`ib_Participant_StartLifecycleWithTime` and
  :cpp:func:`ib_Participant_StartLifecycleNoSyncTime` functions

- Added functionality to remove handlers:

  - ``IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp``

    .. code-block:: c++

      virtual void RemoveSystemStateHandler(HandlerId) = 0;
      virtual void RemoveParticipantStatusHandler(HandlerId) = 0;

  - ``IntegrationBus/include/ib/mw/sync/ITimeProvider.hpp``

    .. code-block:: c++

      virtual void RemoveNextSimStepHandler(HandlerId) = 0;

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    .. code-block:: c++

      virtual void RemoveExplicitDataMessageHandler(HandlerId) = 0;

  - ``IntegrationBus/include/ib/capi/Participant.h``

    .. code-block:: c++

      ib_ReturnCode ib_Participant_RemoveSystemStateHandler(ib_Participant* participant, ib_HandlerId handlerId);
      ib_ReturnCode ib_Participant_RemoveParticipantStatusHandler(ib_Participant* participant, ib_HandlerId handlerId);

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    .. code-block:: c++

      ib_ReturnCode ib_Data_Subscriber_RemoveExplicitDataMessageHandler(ib_Can_Controller* controller, ib_HandlerId handlerId);

Changed
~~~~~~~
- Replaced the participant controller with a life cycle service and a time synchronization service (see documentation for details)
  
  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    + old:
  
      .. code-block:: c++
  
        virtual auto GetParticipantController() -> sync::IParticipantController* = 0;
    + new:
  
      .. code-block:: c++
  
        virtual auto GetLifecycleService() -> sync::ILifecycleService* = 0;

  - The life cycle service comprises methods related to the state control and observation of a participant
  
  - ``IParticipantController::Run()`` was removed
  - ``IParticipantController::RunAsync()`` has two successors
  
    - ``IntegrationBus/include/ib/mw/sync/ILifecycleService.hpp``
  
      + old (life cycle execution):
    
        .. code-block:: c++
    
          virtual auto IParticipantController::RunAsync() -> std::future<ParticipantState> = 0;
          
      + new (life cycle execution):
    
        .. code-block:: c++
  
          virtual auto StartLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
              -> std::future<ParticipantState> = 0;
    
          // corresponds to former functionality of RunAsync()
          virtual auto StartLifecycleWithSyncTime(ITimeSyncService* timeSyncService, bool hasCoordinatedSimulationStart,
                                                  bool hasCoordinatedSimulationStop) -> std::future<ParticipantState> = 0;
  
      + old (callbacks):
    
        .. code-block:: c++
    
          virtual void IParticipantController::CommunicationReadyHandler(CommunicationReadyHandlerT handler) = 0;
          
      + new (callbacks):
    
        .. code-block:: c++
  
          virtual void ILifecycleService::SetCommunicationReadyHandler(CommunicationReadyHandlerT handler) = 0;
          
          // New: indicates transition to ParticipantState::Running for participants without time synchronization
          virtual void SetStartingHandler(StartingHandlerT handler) = 0;

    - Moved methods
    
      + ``IParticipantController::SetStartingHandler(...) -> ILifecycleService::SetStartingHandler(...)``
      + ``IParticipantController::SetStopHandler(...) -> ILifecycleService::SetStopHandler(...)``
      + ``IParticipantController::SetShutdownHandler(...) -> ILifecycleService::SetShutdownHandler(...)``
      + ``IParticipantController::ReportError(...) -> ILifecycleService::ReportError(...)``
      + ``IParticipantController::Pause(...) -> ILifecycleService::Pause(...)``
      + ``IParticipantController::Continue(...) -> ILifecycleService::Continue(...)``
      + ``IParticipantController::Stop(...) -> ILifecycleService::Stop(...)``
      + ``IParticipantController::State(...) -> ILifecycleService::State(...)``
      + ``IParticipantController::Status(...) -> ILifecycleService::Status(...)``

  - The time synchronization service is retrievable via the life cycle service
  - ``IntegrationBus/include/ib/mw/sync/ILifecycleService.hpp``
      .. code-block:: c++
        
        virtual auto GetTimeSyncService() const -> ITimeSyncService* = 0;

  - Moved methods (The time synchronization service methods are unchanged compared to the methods of IParticipantController)
  
    - ``IParticipantController::SetSimulationTask(...) -> ITimeSyncService::SetSimulationTask(...)``
    - ``IParticipantController::SetSimulationTaskAsync(...) -> ITimeSyncService::SetSimulationTaskAsync(...)``
    - ``IParticipantController::CompleteSimulationTask(...) -> ITimeSyncService::CompleteSimulationTask(...)``
    - ``IParticipantController::SetPeriod(...) -> ITimeSyncService::SetPeriod(...)``
    - ``IParticipantController::Now(...) -> ITimeSyncService::Now(...)``
    - ``IParticipantController::SetPeriod(...) -> ITimeSyncService::SetPeriod(...)``
    
- ISystemController: 
  - Shutdown is now a participant command
  
    - ``IntegrationBus/include/ib/mw/sync/ISystemController.hpp``
  
      + old (life cycle execution):
    
        .. code-block:: c++
    
          virtual void Shutdown() const = 0;
          
      + new (life cycle execution):
    
        .. code-block:: c++
  
          virtual void Shutdown(const std::string& participantName) const = 0;
  
  - Renamed reinitialize to restart
  
    - ``IntegrationBus/include/ib/mw/sync/ISystemController.hpp``
  
      + old (life cycle execution):
    
        .. code-block:: c++
    
          virtual void Reinitialize(const std::string& participantName) const = 0;
          
      + new (life cycle execution):
    
        .. code-block:: c++
  
          virtual void Restart(const std::string& participantName) const = 0;

- C-API: renamed the `ib_Participant_WaitForAsyncRunToComplete` to
  `ib_Participant_WaitForLifecycleToComplete`.

- C-API:  the participant Init handler no longer has a command parameter:

  + old:

  .. code-block:: c

    typedef void (*ib_ParticipantCommunicationReadyHandler_t)(void* context,
                      ib_Participant* participant,
                      ib_ParticipantCommand* command);

  + new:

  .. code-block:: c

    typedef void (*ib_ParticipantCommunicationReadyHandler_t)(void* context,
                      ib_Participant* participant);

- SetRequiredParticipants changed to SetWorkflowConfiguration. The new struct currently has the required participants as its sole member.


      + old:

        .. code-block:: c++

            virtual void SetRequiredParticipants(const std::vector<std::string>& participantNames) = 0;

    + new:

        .. code-block:: c++

            virtual void SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration) = 0;

    - ``IntegrationBus/include/ib/capi/Participant.h``:

      + old:

        .. code-block:: c

            typedef ib_ReturnCode(*ib_Participant_SetRequiredParticipants_t)(
                ib_Participant* participant, const ib_StringList* requiredParticipantNames);

    + new:

        .. code-block:: c

            typedef ib_ReturnCode (*ib_Participant_SetWorkflowConfiguration_t)(
                ib_Participant* participant, const ib_WorkflowConfiguration* workflowConfigration);
		
- Methods adding handlers now return a ``HandlerId``:

  - ``IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp``

    .. code-block:: c++

      virtual auto AddSystemStateHandler(SystemStateHandlerT) -> HandlerId = 0;
      virtual auto AddParticipantStatusHandler(ParticipantStatusHandlerT) -> HandlerId = 0;

  - ``IntegrationBus/include/ib/mw/sync/ITimeProvider.hpp``

    .. code-block:: c++

      virtual auto AddNextSimStepHandler(NextSimStepHandlerT) -> HandlerId = 0;

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    .. code-block:: c++

      virtual auto AddExplicitDataMessageHandler(...) -> HandlerId = 0;

  - ``IntegrationBus/include/ib/capi/Participant.h``

    .. code-block:: c++

      ib_ReturnCode ib_Participant_AddSystemStateHandler(..., ib_HandlerId* outHandlerId);
      ib_ReturnCode ib_Participant_AddParticipantStatusHandler(..., ib_HandlerId* outHandlerId);

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    .. code-block:: c++

      ib_ReturnCode ib_Data_Subscriber_AddExplicitDataMessageHandler(..., ib_HandlerId* outHandlerId);

Removed
~~~~~~~
- ISystemController: Removed ``ISystemController::Initialize(const std::string& participantName) const`` 
  without replacement (initialization is perfomed automatically in the new life cycle concept)

- IbSystemControllerInteractive: Removed ``Initialize`` as possible input

- C-API: the  `ib_Participant_RunAsync` is superseded by the
  `ib_Participant_StartLifecycle...` functions.

- C-API: the `ib_Participant_Run` function was removed.
  Use the new asynchronous `ib_Participant_StartLifecycleWithSyncTime` or the
  `ib_Participant_StartLifecycleNoSyncTime` as replacement. For Example:

  + old:

  .. code-block:: c

    ib_ReturnCode returnCode = ib_Participant_Run(participant);

  + new:

  .. code-block:: c

    ib_ReturnCode returnCode = ib_Participant_StartLifecycleNoSyncTime(
                                   participant, ib_False, ib_False, ib_False);
    // error check ommited
    ib_ParticipantState outParticipantState;
    returnCode = ib_Participant_WaitForLifecycleToComplete(participant,
                    &outParticipantState);


[3.99.25] - 2022-06-13
----------------------

Extended Bus System (CAN, Ethernet, FlexRay, ...) APIs and removed separate registry library and unused tooling.

Compatibility with 3.99.24
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): No

Added
~~~~~

- LIN: Added functionality to remove handlers:

  - ``IntegrationBus/include/ib/sim/lin/ILinController.hpp``

    .. code-block:: c++

      virtual HandlerId RemoveFrameStatusHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveGoToSleepHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveWakeupHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveFrameResponseUpdateHandler(HandlerId handlerId) = 0;
      
  - ``IntegrationBus/include/ib/capi/Lin.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Lin_Controller_RemoveFrameStatusHandler_t)(ib_Lin_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Lin_Controller_RemoveGoToSleepHandler_t)(ib_Lin_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Lin_Controller_RemoveWakeupHandler_t)(ib_Lin_Controller* controller, ib_HandlerId handlerId);

- Ethernet: Added functionality to remove handlers:

  - ``IntegrationBus/include/ib/sim/eth/IEthernetController.hpp``

    .. code-block:: c++

      virtual HandlerId RemoveFrameHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveStateChangeHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveFrameTransmitHandler(HandlerId handlerId) = 0;
      
  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveFrameHandler_t)(ib_Ethernet_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveStateChangeHandler_t)(ib_Ethernet_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveFrameTransmitHandler_t)(ib_Ethernet_Controller* controller, ib_HandlerId handlerId);

- Flexray: Added functionality to remove handlers:

  - ``IntegrationBus/include/ib/sim/fr/IFlexrayController.hpp``

    .. code-block:: c++

      virtual HandlerId RemoveFrameHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveFrameTransmitHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveWakeupHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemovePocStatusHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveSymbolHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveSymbolTransmitHandler(HandlerId handlerId) = 0;
      virtual HandlerId RemoveCycleStartHandler(HandlerId handlerId) = 0;

  - ``IntegrationBus/include/ib/capi/Flexray.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveFrameHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveFrameTransmitHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveWakeupHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemovePocStatusHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveSymbolHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveSymbolTransmitHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveCycleStartHandler(ib_Flexray_Controller* controller, ib_HandlerId handlerId);

Removed
~~~~~~~

- The ``vib-config-tool`` has been deprecated and was now finally removed.
  Since the configuration format has been completely reworked, this tool is no longer necessary.
- The ``IbLauncher`` utility has been deprecated and was now finally removed.

Changed
~~~~~~~

- LIN: Adding a handler now returns a HandlerId. In the C-API, the HandlerId is obtaind by an out parameter:

  - ``IntegrationBus/include/ib/sim/lin/ILinController.hpp``

    .. code-block:: c++

      virtual HandlerId AddFrameStatusHandler(...) = 0;
      virtual HandlerId AddGoToSleepHandler(...) = 0;
      virtual HandlerId AddWakeupHandler(...) = 0;
      virtual HandlerId AddFrameResponseUpdateHandler(...) = 0;
      
  - ``IntegrationBus/include/ib/capi/Lin.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Lin_Controller_AddFrameStatusHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Lin_Controller_AddGoToSleepHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Lin_Controller_AddWakeupHandler_t)(... , ib_HandlerId* outHandlerId);

- Ethernet: Adding a handler now returns a HandlerId. In the C-API, the HandlerId is obtaind by an out parameter:

  - ``IntegrationBus/include/ib/sim/eth/IEthernetController.hpp``

    .. code-block:: c++

      virtual HandlerId AddFrameHandler(...) = 0;
      virtual HandlerId AddStateChangeHandler(...) = 0;
      virtual HandlerId AddFrameTransmitHandler(...) = 0;
      
  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Ethernet_Controller_AddFrameHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Ethernet_Controller_AddStateChangeHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Ethernet_Controller_AddFrameTransmitHandler_t)(... , ib_HandlerId* outHandlerId);

- Flexray: Adding a handler now returns a HandlerId. In the C-API, the HandlerId is obtaind by an out parameter:

  - ``IntegrationBus/include/ib/sim/fr/IFlexrayController.hpp``

    .. code-block:: c++

      virtual HandlerId AddFrameHandler(...) = 0;
      virtual HandlerId AddFrameTransmitHandler(...) = 0;
      virtual HandlerId AddWakeupHandler(...) = 0;
      virtual HandlerId AddPocStatusHandler(...) = 0;
      virtual HandlerId AddSymbolHandler(...) = 0;
      virtual HandlerId AddSymbolTransmitHandler(...) = 0;
      virtual HandlerId AddCycleStartHandler(...) = 0;

  - ``IntegrationBus/include/ib/capi/Flexray.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Flexray_Controller_AddFrameHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddFrameTransmitHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddWakeupHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddPocStatusHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddSymbolHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddSymbolTransmitHandler(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Flexray_Controller_AddCycleStartHandler(... , ib_HandlerId* outHandlerId);

- Internal refactoring of Bus Controllers to harmonize behavior w/wo bus simulator.
    
    - LIN: When the controller receives a GoToSleep-frame, the ``FrameStatusHandler`` is always called (previously 
      only with bus simulator).
    - Ethernet: ``Activate()`` and ``Deactivate()`` now tigger the ``StateChangeHandler`` (previously only with bus
      simulator).
    
- The IbRegistry shared library is no longer necessary.
  An instance of IIbRegistry can now be created directly using :cpp:func:`CreateRegistry()<ib::vendor::CreateRegistry>`.
  This is an implementation detail specific to the VAsio based VIB.
  The namespace of the factory function and the location of the headers were changed to reflect this:
   
  + old:
        
  .. code-block:: c++

    //ib/extensions/CreateIbRegistry.hpp
    ib::extensions::CreateIbRegistry()
    
  + new:

  .. code-block:: c++

    //ib/vendor/CreateIbRegistry.hpp
    ib::vendor::CreateIbRegistry()

  The binary packages no longer contain an `IntegrationBus-NonRedistributable` directory.

[3.99.24] - 2022-05-30
----------------------

Refactored Bus System and further Service (CAN, Ethernet, FlexRay, Participant, ...) APIs

Compatibility with 3.99.23
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): No

Added
~~~~~

- ``IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp``

  Added methods which allow users to obtain information about connected participants.

  .. code-block:: c++

    class ISystemMonitor
    {
    public:
        ...
        using ParticipantConnectedHandler = std::function<void(const std::string& participantName)>;
        using ParticipantDisconnectedHandler = std::function<void(const std::string& participantName)>;
        ...
        virtual void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) = 0;
        virtual void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) = 0;
        virtual auto IsParticipantConnected(const std::string& participantName) const -> bool = 0;
        ...
    };

- CAN: Added functionality to remove handlers:

  - ``IntegrationBus/include/ib/sim/can/ICanController.hpp``

    .. code-block:: c++

      virtual void RemoveFrameHandler(HandlerId handlerId) = 0;
      virtual void RemoveFrameTransmitHandler(HandlerId handlerId) = 0;
      virtual void RemoveStateChangeHandler(HandlerId handlerId) = 0;
      virtual void RemoveErrorStateChangeHandler(HandlerId handlerId) = 0;
      
  - ``IntegrationBus/include/ib/capi/Can.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Can_Controller_RemoveFrameHandler_t)(ib_Can_Controller* controller, 
            ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_RemoveFrameTransmitHandler_t)(ib_Can_Controller* controller,
            ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_RemoveStateChangeHandler_t)(ib_Can_Controller* controller,
            ib_HandlerId handlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_RemoveErrorStateChangeHandler_t)(ib_Can_Controller* controller,
            ib_HandlerId handlerId);

Changed
~~~~~~~

- CAN simuations behavior with and without NetSim harmonized: 

  Without NetSim, the ICanController methods Reset, Start, Stop and Sleep now also trigger the 
  StateChangeHandlers on the calling participant, without any effect on the actual controller logic.

- CAN: Adding a handler now returns a HandlerId. In the C-API, the HandlerId is obtaind by an out parameter:

  - ``IntegrationBus/include/ib/sim/can/ICanController.hpp``

    .. code-block:: c++

      virtual HandlerId AddFrameHandler(...) = 0;
      virtual HandlerId AddFrameTransmitHandler(...) = 0;
      virtual HandlerId AddStateChangeHandler(...) = 0;
      virtual HandlerId AddErrorStateChangeHandler(...) = 0;
      
  - ``IntegrationBus/include/ib/capi/Can.h``

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Can_Controller_AddFrameHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_AddFrameTransmitHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_AddStateChangeHandler_t)(... , ib_HandlerId* outHandlerId);
      typedef ib_ReturnCode (*ib_Can_Controller_AddErrorStateChangeHandler_t)(... , ib_HandlerId* outHandlerId);

- Added ib_InterfaceId to structs of C-API:

  + ib_Can_Frame
  + ib_Flexray_ControllerConfig
  + ib_Flexray_HostCommand
  + ib_Flexray_Header
  + ib_Flexray_Frame
  + ib_Flexray_ClusterParameters
  + ib_Flexray_NodeParameters
  + ib_Flexray_TxBufferConfig
  + ib_Flexray_TxBufferUpdate
  + ib_Rpc_DiscoveryResultList

- Changed type of ib_CanErrorState:

  - ``IntegrationBus/include/ib/capi/Can.h``

    + old:

    .. code-block:: c++

      typedef int ib_Can_ErrorState;

    + new:

    .. code-block:: c++

      typedef int32_t ib_Can_ErrorState;

- Changed pass by value semantic in C-API handlers:

  - ``IntegrationBus/include/ib/capi/Can.h``

    + old:

    .. code-block:: c++

      typedef void (*ib_Can_StateChangeHandler_t)(void* context, ib_Can_Controller* controller,
                                             ib_Can_StateChangeEvent stateChangeEvent);
      typedef void (*ib_Can_ErrorStateChangeHandler_t)(void* context, ib_Can_Controller* controller,
                                                  ib_Can_ErrorStateChangeEvent errorStateChangeEvent);

    + new:

    .. code-block:: c++

      typedef void (*ib_Can_StateChangeHandler_t)(void* context, ib_Can_Controller* controller,
                                             ib_Can_StateChangeEvent* stateChangeEvent);
      typedef void (*ib_Can_ErrorStateChangeHandler_t)(void* context, ib_Can_Controller* controller,
                                                  ib_Can_ErrorStateChangeEvent* errorStateChangeEvent);

  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    + old:

    .. code-block:: c++

      typedef void (*ib_Ethernet_StateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
        ib_Ethernet_StateChangeEvent stateChangeEvent);
      typedef void (*ib_Ethernet_BitrateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
        ib_Ethernet_BitrateChangeEvent bitrateChangeEvent);

    + new:

    .. code-block:: c++

      typedef void (*ib_Ethernet_StateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
        ib_Ethernet_StateChangeEvent* stateChangeEvent);
      typedef void (*ib_Ethernet_BitrateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
        ib_Ethernet_BitrateChangeEvent* bitrateChangeEvent);

  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    + old:

    .. code-block:: c++

      typedef void (*ib_ParticipantStatusHandler_t)(void* context, ib_Participant* participant,
        const char* participantName, ib_ParticipantStatus status);

    + new:

    .. code-block:: c++

      typedef void (*ib_ParticipantStatusHandler_t)(void* context, ib_Participant* participant,
        const char* participantName, ib_ParticipantStatus* status);

- Changed ib_Ethernet_Frame C-API:

  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    + old:

    .. code-block:: c++

      typedef ib_ByteVector ib_Ethernet_Frame;

    + new:

    .. code-block:: c++

      typedef struct
        {
            ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
            ib_ByteVector raw;
        } ib_Ethernet_Frame;

- Changed ib_Flexray_ControllerConfig C-API:

  - ``IntegrationBus/include/ib/capi/Flexray.h``

    + old:

    .. code-block:: c++

      struct ib_Flexray_ControllerConfig
        {
            ib_Flexray_ClusterParameters clusterParams;
            ib_Flexray_NodeParameters nodeParams;
            ...

    + new:

    .. code-block:: c++

      struct ib_Flexray_ControllerConfig
        {
            ib_InterfaceIdentifier interfaceId;
            ib_Flexray_ClusterParameters* clusterParams;
            ib_Flexray_NodeParameters* nodeParams;
            ...

[3.99.23] - 25-05-2022
----------------------

Refactored Bus System and further Service (data message, rpc) APIs

Compatibility with 3.99.22
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): Yes


Changed
~~~~~~~

- ``IntegrationBus/include/ib/sim/can/CanDatatypes.hpp``
      
      The two members direction and userContext were moved from the CanFrame to the CanFrameEvent

      + old: 

      .. code-block:: c++

        struct CanFrame
            {
                ...
                TransmitDirection direction{TransmitDirection::Undefined}; //!< Receive/Transmit direction
                void* userContext; //!< Optional pointer provided by user when sending the frame
            };

      + new: 

      .. code-block:: c++

        struct CanFrameEvent
            {
                ...
                TransmitDirection direction{TransmitDirection::Undefined}; //!< Receive/Transmit direction
                void* userContext; //!< Optional pointer provided by user when sending the frame
            };


Removed
~~~~~~~
        
- Removed deprecated PcapFile and PcapPipe config fields in EthernetControllers section. Use UseTraceSinks instead.

- API to read, create and modify Ethernet frames at the ``EthernetFrame`` is removed.

  - ``IntegrationBus/include/ib/sim/eth/EthernetDatatypes.hpp``

    + old: 

    .. code-block:: c++

      struct EthernetTagControlInformation;

      EthernetFrame::EthernetFrame();
      EthernetFrame::EthernetFrame(const EthernetFrame& other);
      EthernetFrame(EthernetFrame&& other);
      auto operator=(const EthernetFrame& other) -> EthernetFrame&;
      auto operator=(EthernetFrame&& other) -> EthernetFrame&;

      EthernetFrame::EthernetFrame(const std::vector<uint8_t>& rawFrame);
      EthernetFrame::EthernetFrame(std::vector<uint8_t>&& rawFrame);
      EthernetFrame::EthernetFrame(const uint8_t* rawFrame, size_t size);
      
      auto EthernetFrame::GetDestinationMac() const -> EthernetMac;
      void EthernetFrame::SetDestinationMac(const EthernetMac& mac);
      auto EthernetFrame::GetSourceMac() const -> EthernetMac;
      void EthernetFrame::SetSourceMac(const EthernetMac& mac);

      auto EthernetFrame::GetVlanTag() const -> EthernetTagControlInformation;
      void EthernetFrame::SetVlanTag(const EthernetTagControlInformation& tci);

      auto EthernetFrame::GetEtherType() const -> uint16_t;
      void EthernetFrame::SetEtherType(uint16_t etherType);

      auto EthernetFrame::GetFrameSize() const -> size_t;
      auto EthernetFrame::GetHeaderSize() const -> size_t;
      auto EthernetFrame::GetPayloadSize() const -> size_t;

      auto EthernetFrame::GetPayload() -> util::vector_view<uint8_t>;
      auto EthernetFrame::GetPayload() const -> util::vector_view<const uint8_t>;
      void EthernetFrame::SetPayload(const std::vector<uint8_t>& payload);
      void EthernetFrame::SetPayload(const uint8_t* payload, size_t size);

      auto EthernetFrame::RawFrame() const -> const std::vector<uint8_t>&;
      void EthernetFrame::SetRawFrame(const std::vector<uint8_t>&);

    + new:

    .. code-block:: c++

      struct EthernetFrame
      {
          std::vector<uint8_t> raw;
      };

  - Removed deprecated PcapFile and PcapPipe config fields in EthernetControllers section. Use UseTraceSinks instead.

  - Removed MacAddress config fields in EthernetControllers section.

Fixed
~~~~~~~

  - Removed bug that allowed for multiple parallel SimTask-Handle triggers without a call to CompleteSimulationTask when using ParticipantController::RunAsync


[3.99.22] - 2022-05-17
----------------------

Refactored Bus System and further Service (data message, rpc) APIs

Compatibility with 3.99.21
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): No

Removed
~~~~~~~

- ``IntegrationBus/include/ib/version.hpp``
    
  The function to retreive the Sprint name was removed. The CMake-Variables ``IB_SPRINT_NUMBER`` and
  ``IB_SPRINT_NAME`` were removed as well.

      + old: 

    .. code-block:: c++
          
        ib::version::SprintName()

- ``IntegrationBus/include/ib/capi/FlexRay.h``
      
    The convenience function in the C-API to append a ``TxBufferConfig`` was removed. 

    + old: 

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_FlexRay_Append_TxBufferConfig_t)(ib_FlexRay_ControllerConfig** controllerConfig, 
        const ib_FlexRay_TxBufferConfig* txBufferConfig);

Changed
~~~~~~~
- The IbRegistry utility changed the configuration parameter from positional parameter to option parameter.

  + old: 

  .. code-block:: powershell

    ./IbRegistry IbConfig_DemoCan.json

  + new: 

  .. code-block:: powershell

    ./IbRegistry -c IbConfig_DemoCan.json

- Dynamic arrays in C-API changed from *array of size 1 at end of struct* to pointers:
  
  - ``IntegrationBus/include/ib/capi/FlexRay.h``

    + old: 

    .. code-block:: c++

      struct ib_FlexRay_ControllerConfig {
        ...
        ib_FlexRay_TxBufferConfig bufferConfigs[1];
      };

    + new: 

    .. code-block:: c++

      struct ib_FlexRay_ControllerConfig {
        ...
        ib_FlexRay_TxBufferConfig* bufferConfigs;
      };

  - ``IntegrationBus/include/ib/capi/Lin.h``

    + old: 

    .. code-block:: c++

      struct ib_Lin_ControllerConfig {
        ...
        ib_Lin_FrameResponse frameResponses[1];
      };

    + new: 

    .. code-block:: c++

      struct ib_Lin_ControllerConfig {
        ...
        ib_Lin_FrameResponse* frameResponses;
      };

  - ``IntegrationBus/include/ib/capi/Rpc.h``

    + old:

    .. code-block:: c++

      typedef struct ib_Rpc_DiscoveryResultList
      {
        ...
        ib_Rpc_DiscoveryResult results[1];
      } ib_Rpc_DiscoveryResultList;

    + new: 

    .. code-block:: c++

      typedef struct ib_Rpc_DiscoveryResultList
      {
        ...
        ib_Rpc_DiscoveryResult* results;
      } ib_Rpc_DiscoveryResultList;


  - ``IntegrationBus/include/ib/capi/Types.h``

    + old:

    .. code-block:: c++

      typedef struct ib_KeyValueList
      {
        size_t numLabels;
        ib_KeyValuePair labels[1];
      } ib_KeyValueList;
      
      typedef struct ib_StringList
      {
        size_t numStrings;
        const char* strings[1];
      } ib_StringList;

    + new:

    .. code-block:: c++

      typedef struct ib_KeyValueList
      {
        size_t numLabels;
        ib_KeyValuePair* labels;
      } ib_KeyValueList;
      
      typedef struct ib_StringList
      {
        size_t numStrings;
        const char** strings;
      } ib_StringList;


- Can

  - ``IntegrationBus/include/ib/sim/can/CanDatatypes.hpp``

    + old:

    .. code-block:: c++

      struct CanReceiveFlags

    + new:

    .. code-block:: c++

      struct CanFrameFlags

- Lin

  "Lin"-prefix for related data types.

  - ``IntegrationBus/include/ib/sim/lin/LinDatatypes.hpp``

    + old:

    .. code-block:: c++
      
      ChecksumModel
      DataLengthT
      FrameResponseType
      FrameResponseMode
      FrameResponse
      FrameStatus
      ControllerMode
      BaudRateT
      ControllerConfig
      ControllerStatus

    + new:

    .. code-block:: c++
      
      LinChecksumModel
      LinDataLengthT
      LinFrameResponseType
      LinFrameResponseMode
      LinFrameResponse
      LinFrameStatus
      LinControllerMode
      LinBaudRateT
      LinControllerConfig
      LinControllerStatus

- Replaced the single-member struct ``RpcExchangeFormat`` with its sole member, the media type string.
  The related data types were removed and some associated functions have changed:

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    + old

    .. code-block:: c++

      class IParticipant
          auto CreateRpcClient(..., RpcExchangeFormat exchangeFormat, ...) -> ...
          auto CreateRpcServer(..., RpcExchangeFormat exchangeFormat, ...) -> ...
          auto DiscoverRpcServers(..., RpcExchangeFormat exchangeFormat, ...) -> ...

    + new

    .. code-block:: c++

      class IParticipant
          auto CreateRpcClient(..., const std::string& mediaType, ...) -> ...
          auto CreateRpcServer(..., const std::string& mediaType, ...) -> ...
          auto DiscoverRpcServers(..., const std::string& mediaType, ...) -> ...

  - ``IntegrationBus/include/ib/sim/rpc/RpcDatatypes.hpp``

    + old

    .. code-block:: c++

      struct RpcExchangeFormat { ... };
      bool operator ==(const RpcExchangeFormat &, const RpcExchangeFormat &);

  - ``IntegrationBus/include/ib/sim/rpc/string_utils.hpp``

    + old

    .. code-block:: c++

      to_string(const RpcExchangeFormat&) -> std::string;
      operator<<(std::ostream& out, const RpcExchangeFormat& dataExchangeFormat) -> std::ostream&;

  - ``IntegrationBus/include/ib/capi/InterfaceIdentifiers.h``

    + old

    .. code-block:: c++

      #define ib_InterfaceIdentifier_RpcExchangeFormat ...

  - ``IntegrationBus/include/ib/capi/Rpc.h``

    + old

    .. code-block:: c++

      typedef struct { ... } ib_Rpc_ExchangeFormat;

- FlexRay

  - Renamed ``IntegrationBus/include/ib/capi/FlexRay.h`` to ``IntegrationBus/include/ib/capi/Flexray.h``

    - Changed ``ib_FlexRay_`` to ``ib_Flexray_`` in all symbols

    - Changed the names of the event and handler types and registration functionsto match the ``...Event``,
      ``...TransmitEvent`` and ``Add...Handler`` naming scheme

    + old:

    .. code-block:: c++

      ib_FlexRay_Message
      ib_FlexRay_MessageAck
      ib_FlexRay_Symbol
      ib_FlexRay_SymbolAck
      ib_FlexRay_CycleStart
      ib_FlexRay_ControllerStatus
      ib_FlexRay_PocStatus
      ib_FlexRay_MessageHandler_t
      ib_FlexRay_MessageAckHandler_t
      ib_FlexRay_WakeupHandler_t
      ib_FlexRay_PocStatusHandler_t
      ib_FlexRay_SymbolHandler_t
      ib_FlexRay_SymbolAckHandler_t
      ib_FlexRay_CycleStartHandler_t
      ib_FlexRay_Controller_RegisterMessageHandler
      ib_FlexRay_Controller_RegisterMessageAckHandler
      ib_FlexRay_Controller_RegisterWakeupHandler
      ib_FlexRay_Controller_RegisterPocStatusHandler
      ib_FlexRay_Controller_RegisterSymbolHandler
      ib_FlexRay_Controller_RegisterSymbolAckHandler
      ib_FlexRay_Controller_RegisterCycleStartHandler

    + new:

    .. code-block:: c++

      ib_Flexray_FrameEvent
      ib_Flexray_FrameTransmitEvent
      ib_Flexray_SymbolEvent
      ib_Flexray_SymbolTransmitEvent
      ib_Flexray_WakeupEvent
      ib_Flexray_CycleStartEvent
      ib_Flexray_PocStatusEvent
      ib_Flexray_FrameHandler_t
      ib_Flexray_FrameTransmitHandler_t
      ib_Flexray_WakeupHandler_t
      ib_Flexray_PocStatusHandler_t
      ib_Flexray_SymbolHandler_t
      ib_Flexray_SymbolTransmitHandler_t
      ib_Flexray_CycleStartHandler_t
      ib_Flexray_Controller_AddFrameHandler
      ib_Flexray_Controller_AddFrameTransmitHandler
      ib_Flexray_Controller_AddWakeupHandler
      ib_Flexray_Controller_AddPocStatusHandler
      ib_Flexray_Controller_AddSymbolHandler
      ib_Flexray_Controller_AddSymbolTransmitHandler
      ib_Flexray_Controller_AddCycleStartHandler

  - ``IntegrationBus/include/ib/capi/InterfaceIdentifiers.h``

    - Changed ``FlexRay`` to ``Flexray`` in all names

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    - Changed ``Fr`` to ``Flexray`` in all names

  - Renamed ``IntegrationBus/include/ib/sim/fr/FrDatatypes.hpp`` to ``IntegrationBus/include/ib/sim/fr/FlexrayDatatypes.hpp``

    - Changed ``Fr`` to ``Flexray`` in all names

    - Added ``Flexray`` prefix to all names which had no prefix

    - Changed the names of the event and handler types and registration functionsto match the ``...Event``,
      ``...TransmitEvent`` and ``Add...Handler`` naming scheme

    + old

    .. code-block:: c++

      FrMessage
      FrMessageAck
      FrSymbol
      FrSymbolAck
      CycleStart
      PocStatus

    + new

    .. code-block:: c++

      FlexrayFrameEvent
      FlexrayFrameTransmitEvent
      FlexraySymbolEvent
      FlexraySymbolTransmitEvent
      FlexrayCycleStartEvent
      FlexrayPocStatusEvent

  - Renamed ``IntegrationBus/include/ib/sim/fr/IFrController.hpp`` to ``IntegrationBus/include/ib/sim/fr/IFlexrayController.hpp``

    - Changed ``Fr`` to ``Flexray`` in all names

    - Changed the names of the event and handler types and registration functionsto match the ``...Event``,
      ``...TransmitEvent`` and ``Add...Handler`` naming scheme

    + old

    .. code-block:: c++

      class IFrController
        MessageHandler
        MessageAckHandler
        SymbolAckHandler
        RegisterMessageHandler
        RegisterMessageAckHandler
        RegisterWakeupHandler
        RegisterPocStatusHandler
        RegisterSymbolHandler
        RegisterSymbolAckHandler
        RegisterCycleStartHandler

    + new

    .. code-block:: c++

      class IFlexrayController
        FrameHandler
        FrameTransmitHandler
        SymbolTransmitHandler
        AddFrameHandler
        AddFrameTransmitHandler
        AddWakeupHandler
        AddPocStatusHandler
        AddSymbolHandler
        AddSymbolTransmitHandler
        AddCycleStartHandler

- RPC

  - ``IntegrationBus/include/ib/capi/Rpc.h``

    The individual parameters of the call and call-result handlers were combined into a single event structure.
    The handler typedefs were renamed to be in line with the corresponding ``C++`` API

    + old

    .. code-block:: c++

      typedef void (*ib_Rpc_CallHandler_t)(void* context, ib_Rpc_Server* server, ib_Rpc_CallHandle* callHandle, const ib_ByteVector* argumentData);
      typedef void (*ib_Rpc_ResultHandler_t)(void* context, ib_Rpc_Client* client, ib_Rpc_CallHandle* callHandle, ib_Rpc_CallStatus callStatus, const ib_ByteVector* returnData);

    + new

    .. code-block:: c++

      typedef void (*ib_Rpc_CallHandler_t)(void* context, ib_Rpc_Server* server, const ib_Rpc_CallEvent* event);
      typedef void (*ib_Rpc_CallResultHandler_t)(void* context, ib_Rpc_Client* client, const ib_Rpc_CallResultEvent* event);

    The former ``rpcChannel`` was renamed to ``functionName`` which should better reflect it's meaning:

    + old

    .. code-block:: c++

      typedef struct ib_Rpc_DiscoveryResult
      {
          ...
          const char* rpcChannel;
          ...
      } ib_Rpc_DiscoveryResult;

    + new

    .. code-block:: c++

      typedef struct ib_Rpc_DiscoveryResult
      {
          ...
          const char* functionName;
          ...
      } ib_Rpc_DiscoveryResult;

  - ``IntegrationBus/include/ib/sim/rpc/RpcDatatypes.hpp``

    The type ``ib::sim::rpc::CallStatus`` was renamed to ``ib::sim::rpc::RpcCallStatus``.

    The typedef ``CallReturnHandler`` was renamed to ``CallResultHandler`` and the arguments besides the ``IRpcClient*`` were combined into an event structure:

    + old

    .. code-block:: c++

      using CallReturnHandler = std::function<void(ib::sim::rpc::IRpcClient* client,
                                                   ib::sim::rpc::IRpcCallHandle* callHandle,
                                                   const ib::sim::rpc::CallStatus callStatus,
                                                   const std::vector<uint8_t>& returnData)>;

    + new

    .. code-block:: c++

      struct RpcCallResultEvent
      {
          std::chrono::nanoseconds timestamp;
          IRpcCallHandle* callHandle;
          RpcCallStatus callStatus;
          std::vector<uint8_t> resultData;
      };

      using RpcCallResultHandler = std::function<void(IRpcClient* client, const RpcCallResultEvent& event)>;

    The typedef ``CallProcessor`` was renamed to ``CallHandler``.

    + old

    .. code-block:: c++

      using CallProcessor = std::function<void(ib::sim::rpc::IRpcServer* server,
                                               ib::sim::rpc::IRpcCallHandle* callHandle,
                                               const std::vector<uint8_t>& argumentData)>;

    + new

    .. code-block:: c++

      struct RpcCallEvent
      {
          std::chrono::nanoseconds timestamp;
          IRpcCallHandle* callHandle;
          std::vector<uint8_t> argumentData;
      };

      using RpcCallHandler = std::function<void(IRpcServer* server, const RpcCallEvent& event)>;

    The typedef ``DiscoveryResultHandler`` was renamed to ``RpcDiscoveryResultHandler``.

  - The ``Channel`` field of the ``RpcClients`` and ``RpcServers`` entries in the participant configuration was renamed
    to ``FunctionName``.

Added
~~~~~

- Lin

  Improved error handling for Wakeup/GoToSleep.

  - ``IntegrationBus/include/ib/sim/lin/ILinController.hpp``

    ``ILinController::GoToSleep()``, ``ILinController::GoToSleepInternal()``, ``ILinController::Wakeup()`` and 
    ``ILinController::WakeupInternal()`` now throw a ``ib::StateError`` exception if issued in wrong 
    ``LinControllerMode``.

  - ``IntegrationBus/include/ib/capi/Lin.h``
  
    ``ib_Lin_Controller_GoToSleep()``, ``ib_Lin_Controller_GoToSleepInternal()``, ``ib_Lin_Controller_Wakeup()`` and 
    ``ib_Lin_Controller_WakeupInternal()`` now return ``ib_ReturnCode_WRONGSTATE`` if issued in wrong 
    ``ib_Lin_ControllerMode``.

- FlexRay

  - ``IntegrationBus/include/ib/capi/InterfaceIdentifiers.h``

    New interface identifier for wakeup events

    + new:

    .. code-block:: c++

      #define ib_InterfaceIdentifier_FlexrayWakeupEvent ...

  - ``IntegrationBus/include/ib/sim/fr/FlexrayDatatypes.hpp``

    New datatype for wakeup events

    + new

    .. code-block:: c++

      struct FlexrayWakeupEvent { ... };

- RPC

  - ``IntegrationBus/include/ib/capi/Rpc.h``

    Functions to set the handler for an existing RPC client and server:

    + new

    .. code-block:: c++

      ib_ReturnCode ib_Rpc_Server_SetCallHandler(ib_Rpc_Server* self, void* context, ib_Rpc_CallHandler_t handler);
      ib_ReturnCode ib_Rpc_Client_SetCallResultHandler(ib_Rpc_Client* self, void* context, ib_Rpc_CallResultHandler_t handler);

    Event structures that are used instead of the individual parameters of the handler callbacks:

    + new

    .. code-block:: c++

      typedef struct {
          ib_InterfaceIdentifier interfaceId;
          ib_NanosecondsTime timestamp;
          ib_Rpc_CallHandle* callHandle;
          ib_ByteVector argumentData;
      } ib_Rpc_CallEvent;

      typedef struct {
          ib_InterfaceIdentifier interfaceId;
          ib_NanosecondsTime timestamp;
          ib_Rpc_CallHandle* callHandle;
          ib_Rpc_CallStatus callStatus;
          ib_ByteVector resultData;
      } ib_Rpc_CallResultEvent;

  - ``IntegrationBus/include/ib/capi/InterfaceIdentifiers.h``

    Added interface identifiers for the newly introduced event structures.

    + new

    .. code-block:: c++

      #define ib_InterfaceIdentifier_RpcCallEvent ...
      #define ib_InterfaceIdentifier_RpcCallResultEvent ...

Fixed
~~~~~
- CAN: fixed network transmission of the userContext member. This breaks network compatibility to
  the previous 3.99.21 release.



[3.99.21] - 2022-05-03
----------------------

Compatibility with 3.99.20
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (VAsio): No

Changed
~~~~~~~

- Lin

  - Functional changes
  
    - Sending a wakeup pulse ´´ILinController::Wakeup()´´ now also triggers all ``WakeupHandler``-callbacks on the controller that initiated the
      wakeup pulse in a trivial simulation. Formerly, this was only the case in a detailed simulation. The direction can be distinguished with
      the new ´´LinWakeupEvent.direction´´.

  - ``IntegrationBus/include/ib/sim/lin/ILinController.hpp``

    + old:

    .. code-block:: c++

      using FrameStatusHandler = std::function<void(ILinController*, const LinFrame&, FrameStatus, std::chrono::nanoseconds timestamp)>;
      using WakeupHandler = std::function<void(ILinController*)>;
      using GoToSleepHandler = std::function<void(ILinController*)>;

      ILinController::RegisterFrameStatusHandler(FrameStatusHandler); 
      ILinController::RegisterGoToSleepHandler(GoToSleepHandler); 
      ILinController::RegisterWakeupHandler(WakeupHandler); 
      ILinController::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler); 

    + new:

    .. code-block:: c++

      using FrameStatusHandler = std::function<void(ILinController*, const LinFrameStatusEvent& frameStatusEvent)>;
      using WakeupHandler = std::function<void(ILinController*, const LinWakeupEvent& wakeupEvent)>;
      using GoToSleepHandler = std::function<void(ILinController*, const LinGoToSleepEvent& goToSleepEvent)>;

      ILinController::AddFrameStatusHandler(FrameStatusHandler); 
      ILinController::AddGoToSleepHandler(GoToSleepHandler);
      ILinController::AddWakeupHandler(WakeupHandler); 
      ILinController::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler); 
      
  - ``IntegrationBus/include/ib/sim/lin/LinDatatypes.hpp`` (C++-Api)

    + old: 

    .. code-block:: c++
    
      struct Frame {...};

    + new:

    .. code-block:: c++

      struct LinFrame {...};

    + added:

    .. code-block:: c++
    
      struct LinFrameStatusEvent
      {
          std::chrono::nanoseconds timestamp;
          const LinFrame& frame;
          FrameStatus status;
      };

      struct LinWakeupEvent
      {
          std::chrono::nanoseconds timestamp;
          TransmitDirection direction;
      };

      struct LinGoToSleepEvent
      {
          std::chrono::nanoseconds timestamp;
      };

  - ``IntegrationBus/include/ib/capi/Lin.h``

    - Data types

      + added:

      .. code-block:: c++

        struct ib_Lin_FrameStatusEvent
        {
            ib_InterfaceIdentifier interfaceId;
            ib_NanosecondsTime timestamp;
            ib_Lin_Frame* frame;
            ib_Lin_FrameStatus status;
        };

        struct ib_Lin_WakeupEvent
        {
            ib_InterfaceIdentifier interfaceId;
            ib_NanosecondsTime timestamp; 
            ib_Direction direction;
        };

        struct ib_Lin_GoToSleepEvent
        {
            ib_InterfaceIdentifier interfaceId;
            ib_NanosecondsTime timestamp;
        };

    - Handlers

      + old: 

      .. code-block:: c++
      
        typedef void (*ib_Lin_FrameStatusHandler_t)(void* context, ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
          ib_Lin_FrameStatus status, ib_NanosecondsTime timestamp);
          
        typedef void (*ib_Lin_GoToSleepHandler_t)(void* context, ib_Lin_Controller* controller);

        typedef void (*ib_Lin_WakeupHandler_t)(void* context, ib_Lin_Controller* controller);
                           
      + new:

      .. code-block:: c++
        
        typedef void (*ib_Lin_FrameStatusHandler_t)(void* context, ib_Lin_Controller* controller,
          const ib_Lin_FrameStatusEvent* frameStatusEvent);

        typedef void (*ib_Lin_GoToSleepHandler_t)(void* context, ib_Lin_Controller* controller,
          const ib_Lin_GoToSleepEvent* goToSleepEvent);

        typedef void (*ib_Lin_WakeupHandler_t)(void* context, ib_Lin_Controller* controller, 
          const ib_Lin_WakeupEvent* wakeUpEvent);


    - Methods

      + old: 

      .. code-block:: c++
      
        typedef ib_ReturnCode(*ib_Lin_Controller_RegisterFrameStatusHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_FrameStatusHandler_t handler);
          
        typedef ib_ReturnCode(*ib_Lin_Controller_RegisterGoToSleepHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_GoToSleepHandler_t handler);  
          
        typedef ib_ReturnCode(*ib_Lin_Controller_RegisterWakeupHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_WakeupHandler_t handler); 
          
        typedef ib_ReturnCode(*ib_Lin_Controller_SetFrameResponse_t)(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
          ib_Lin_FrameResponseMode mode);
  

      + new:

      .. code-block:: c++
      
        typedef ib_ReturnCode(*ib_Lin_Controller_AddFrameStatusHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_FrameStatusHandler_t handler);
          
        typedef ib_ReturnCode(*ib_Lin_Controller_AddGoToSleepHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_GoToSleepHandler_t handler);  

        typedef ib_ReturnCode(*ib_Lin_Controller_AddWakeupHandler_t)(ib_Lin_Controller* controller, void* context,
          ib_Lin_WakeupHandler_t handler); 
          
        typedef ib_ReturnCode(*ib_Lin_Controller_SetFrameResponse_t)(ib_Lin_Controller* controller, 
          const ib_Lin_FrameResponse* frameResponse);

- Can

  - ``IntegrationBus/include/ib/sim/can/ICanController.hpp``

    + old:

    .. code-block:: c++

      using ReceiveMessageHandler    = CallbackT<CanMessage>;
      using StateChangedHandler      = CallbackT<CanState>;
      using ErrorStateChangedHandler = CallbackT<CanErrorState>;
      using MessageStatusHandler     = CallbackT<CanTransmitAcknowledge>;

      ICanController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler);
      ICanController::RegisterStateChangedHandler(StateChangedHandler handler);
      ICanController::RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler);
      ICanController::RegisterTransmitStatusHandler(MessageAckHandler handler);
      ICanController::SendMessage(const CanMessage& msg, void* userContext = nullptr) -> CanTxId;

    + new:

    .. code-block:: c++

      using FrameHandler             = CallbackT<CanFrameEvent>;
      using StateChangeHandler       = CallbackT<CanStateChangeEvent>;
      using ErrorStateChangeHandler  = CallbackT<CanErrorStateChangeEvent>;
      using FrameTransmitHandler     = CallbackT<CanFrameTransmitEvent>;

      ICanController::AddFrameHandler(FrameHandler handler);
      ICanController::AddStateChangeHandler(StateChangeHandler handler);
      ICanController::AddErrorStateChangeHandler(ErrorStateChangeHandler handler);
      ICanController::AddFrameTransmitHandler(FrameTransmitHandler handler);
      ICanController::SendFrame(const CanFrame& msg, void* userContext = nullptr) -> CanTxId;

  - ``IntegrationBus/include/ib/sim/can/CanDatatypes.hpp`` (C++-Api)

    + old: 

    .. code-block:: c++

      struct CanMessage
      {
          CanTxId transmitId; //!< Set by the CanController, used for acknowledgements
          std::chrono::nanoseconds timestamp; //!< Reception time

          // CAN message content
          ...
      };

      struct CanTransmitAcknowledge {...};

    + new:

    .. code-block:: c++

      struct CanFrame
      {
          // CAN frame content
          ...
      };

      struct CanFrameTransmitEvent {...};

    + added:

    .. code-block:: c++
      
      struct CanFrameEvent
      {
          CanTxId transmitId; //!< Set by the CanController, used for acknowledgements
          std::chrono::nanoseconds timestamp; //!< Send time
          CanFrame frame; //!< The incoming CAN Frame
      };
      
      struct CanStateChangeEvent
      {
          std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
          CanControllerState state; //!< The new state
      };
      
      struct CanErrorStateChangeEvent
      {
          std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
          CanErrorState errorState; //!< The new error state
      };

  - ``IntegrationBus/include/ib/capi/Can.h``

    - Data types

      + old: 

      .. code-block:: c++
      
        struct ib_Can_Frame
        {
            ...
            uint32_t flags; //!< CAN Arbitration and Control Field Flags; see ib_Can_MessageFlag
            ...
        };

        struct ib_Can_Message{...};
        struct ib_Can_TransmitAcknowledge{...};
      
      + new:

      .. code-block:: c++
      
        struct ib_Can_Frame
        {
            ...
            ib_Can_FrameFlag flags; //!< CAN Arbitration and Control Field Flags; see ib_Can_FrameFlag
            ...
        };

        struct ib_Can_FrameEvent{...};
        struct ib_Can_FrameTransmitEvent{...};
      
      + added:

      .. code-block:: c++

        struct ib_Can_StateChangeEvent
        {
            ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
            ib_NanosecondsTime timestamp; //!< Reception time
            ib_Can_ControllerState state; //!< CAN controller state
        };

        struct ib_Can_ErrorStateChangeEvent
        {
            ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
            ib_NanosecondsTime timestamp; //!< Reception time
            ib_Can_ErrorState errorState; //!< CAN controller error state
        };

    - Handlers

      + old: 

      .. code-block:: c++
      
        typedef void (*ib_Can_TransmitStatusHandler_t)(void* context, ib_Can_Controller* controller, 
          ib_Can_TransmitAcknowledge* acknowledge);
      
        typedef void (*ib_Can_ReceiveMessageHandler_t)(void* context, ib_Can_Controller* controller, 
          ib_Can_Message* metaData);

        typedef void (*ib_Can_StateChangedHandler_t)(void* context, ib_Can_Controller* controller, 
          ib_Can_ControllerState state);
        
        typedef void (*ib_Can_ErrorStateChangedHandler_t)(void* context, ib_Can_Controller* controller, 
          ib_Can_ErrorState errorState);

      + new:

      .. code-block:: c++

        typedef void (*ib_Can_FrameTransmitHandler_t)(void* context, ib_Can_Controller* controller,
          ib_Can_FrameTransmitEvent* frameTransmitEvent);

        typedef void (*ib_Can_FrameHandler_t)(void* context, ib_Can_Controller* controller,
          ib_Can_FrameEvent* frameEvent);

        typedef void (*ib_Can_StateChangeHandler_t)(void* context, ib_Can_Controller* controller,
          ib_Can_StateChangeEvent stateChangeEvent);

        typedef void (*ib_Can_ErrorStateChangeHandler_t)(void* context, ib_Can_Controller* controller,
          ib_Can_ErrorStateChangeEvent errorStateChangeEvent);
      
    - Methods

      + old: 

      .. code-block:: c++

        typedef ib_ReturnCode (*ib_Can_Controller_RegisterTransmitStatusHandler_t)(
          ib_Can_Controller* controller, 
          void* context, 
          ib_Can_TransmitStatusHandler_t handler,
          ib_Can_TransmitStatus statusMask);

        typedef ib_ReturnCode (*ib_Can_Controller_RegisterReceiveMessageHandler_t)(ib_Can_Controller* controller, void* context, 
          ib_Can_ReceiveMessageHandler_t handler);

        typedef ib_ReturnCode (*ib_Can_Controller_RegisterStateChangedHandler_t)(ib_Can_Controller* controller, void* context, 
          ib_Can_StateChangedHandler_t handler);

        typedef ib_ReturnCode (*ib_Can_Controller_RegisterErrorStateChangedHandler_t)(ib_Can_Controller* controller, 
          void* context, ib_Can_ErrorStateChangedHandler_t handler);

      + new:

      .. code-block:: c++
      
        typedef ib_ReturnCode (*ib_Can_Controller_AddFrameTransmitHandler_t)(
          ib_Can_Controller* controller, 
          void* context, 
          ib_Can_FrameTransmitHandler_t handler,
          ib_Can_TransmitStatus statusMask);

        typedef ib_ReturnCode (*ib_Can_Controller_AddFrameHandler_t)(ib_Can_Controller* controller, void* context, 
          ib_Can_FrameHandler_t handler);

        typedef ib_ReturnCode (*ib_Can_Controller_AddStateChangeHandler_t)(ib_Can_Controller* controller, void* context, 
          ib_Can_StateChangeHandler_t handler);

        typedef ib_ReturnCode (*ib_Can_Controller_AddErrorStateChangeHandler_t)(ib_Can_Controller* controller, 
          void* context, ib_Can_ErrorStateChangeHandler_t handler);

- Ethernet

  - Renamed files

    - ``IntegrationBus/include/ib/sim/eth/EthDatatypes.hpp`` to ``EthernetDatatypes.hpp``
    - ``IntegrationBus/include/ib/sim/eth/IEthController.hpp`` to ``IEthernetController.hpp``

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``
  
    + old:

    .. code-block:: c++

      IParticipant::CreateEthController(const std::string& canonicalName, const std::string& networkName)
      IParticipant::CreateEthController(const std::string& canonicalName) -> sim::eth::IEthController*;

    + new:

    .. code-block:: c++

      IParticipant::CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
      IParticipant::CreateEthernetController(const std::string& canonicalName) -> sim::eth::IEthernetController*;

  - ``IntegrationBus/include/ib/sim/eth/IEthController.hpp``

    + old:

    .. code-block:: c++

      using ReceiveMessageHandler = CallbackT<EthMessage>;
      using MessageAckHandler     = CallbackT<EthTransmitAcknowledge>;
      using StateChangedHandler   = CallbackT<EthState>;
      using BitRateChangedHandler = CallbackT<uint32_t>;

      IEthController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler);
      IEthController::RegisterMessageAckHandler(MessageAckHandler handler);
      IEthController::RegisterStateChangedHandler(StateChangedHandler handler);
      IEthController::RegisterBitRateChangedHandler(BitRateChangedHandler handler);
      IEthController::SendFrame(EthFrame msg) -> EthTxId;

    + new:

    .. code-block:: c++

      using FrameHandler         = CallbackT<EthernetFrameEvent>;
      using FrameTransmitHandler = CallbackT<EthernetFrameTransmitEvent>;
      using StateChangeHandler   = CallbackT<EthernetStateChangeEvent>;
      using BitrateChangeHandler = CallbackT<EthernetBitrateChangeEvent>;

      IEthernetController::AddFrameHandler(FrameHandler handler);
      IEthernetController::AddFrameTransmitHandler(FrameTransmitHandler handler);
      IEthernetController::AddStateChangeHandler(StateChangeHandler handler);
      IEthernetController::AddBitrateChangeHandler(BitrateChangeHandler handler);
      IEthernetController::SendFrame(EthernetFrame msg) -> EthernetTxId;

  - ``IntegrationBus/include/ib/sim/eth/EthDatatypes.hpp`` (C++-Api)
    
    For clarity, only the renaming overview is given in *old* and *new*. Internal member names have changed accordingly.
    Additional data types follow in *added*.

    + old: 

    .. code-block:: c++

      EthMac
      EthVid
      EthTagControlInformation
      EthFrame
      EthTxId 
      EthMessage
      EthTransmitStatus
      EthTransmitAcknowledge
      EthState
      EthStatus
      EthMode
      EthSetMode

    + new:

    .. code-block:: c++

      EthernetMac
      EthernetVid
      EthernetTagControlInformation
      EthernetFrame
      EthernetTxId
      EthernetFrameEvent
      EthernetTransmitStatus
      EthernetFrameTransmitEvent
      EthernetState
      EthernetStatus
      EthernetMode
      EthernetSetMode

    + added:

    .. code-block:: c++
      
      struct EthernetStateChangeEvent
      {
          std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
          EthernetState state; //!< State of the Ethernet controller.
      };

      using EthernetBitrate = uint32_t;
      
      struct EthernetBitrateChangeEvent
      {
          std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
          EthernetBitrate bitrate; //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
      };

  - ``IntegrationBus/include/ib/capi/Ethernet.h``

    - Data types

      + old: 

      .. code-block:: c++
      
        ib_Ethernet_Message
        ib_Ethernet_TransmitAcknowledge
      
      + new:

      .. code-block:: c++
      
        ib_Ethernet_FrameEvent
        ib_Ethernet_FrameTransmitEvent
      
      + added:

      .. code-block:: c++


        typedef struct 
        {
            ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
            ib_NanosecondsTime timestamp; //!< Timestamp of the state change event
            ib_Ethernet_State state; //!< New state of the Ethernet controller
        } ib_Ethernet_StateChangeEvent;

        typedef uint32_t ib_Ethernet_Bitrate; //!< Bitrate in kBit/sec

        typedef struct
        {
            ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
            ib_NanosecondsTime timestamp; //!< Timestamp of the bitrate change event
            ib_Ethernet_Bitrate bitrate; //!< New bitrate in kBit/sec
        } ib_Ethernet_BitrateChangeEvent;

    - Handlers

      + old: 

      .. code-block:: c++
      
        typedef void (*ib_Ethernet_ReceiveMessageHandler_t)(void* context, ib_Ethernet_Controller* controller, 
            ib_Ethernet_Message* message);
      
        typedef void (*ib_Ethernet_FrameAckHandler_t)(void* context, ib_Ethernet_Controller* controller, 
            ib_Ethernet_TransmitAcknowledge* acknowledge);
      
        typedef void (*ib_Ethernet_StateChangedHandler_t)(void* context, ib_Ethernet_Controller* controller,
            ib_Ethernet_State state);
      
        typedef void (*ib_Ethernet_BitRateChangedHandler_t)(void* context, ib_Ethernet_Controller* controller,
            uint32_t bitrate);
      
      + new:

      .. code-block:: c++
      
        typedef void (*ib_Ethernet_FrameHandler_t)(void* context, ib_Ethernet_Controller* controller, 
            ib_Ethernet_FrameEvent* frameEvent);
      
        typedef void (*ib_Ethernet_FrameTransmitHandler_t)(void* context, ib_Ethernet_Controller* controller, 
            ib_Ethernet_FrameTransmitEvent* frameTransmitEvent);
      
        typedef void (*ib_Ethernet_StateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
            ib_Ethernet_StateChangeEvent stateChangeEvent);
      
        typedef void (*ib_Ethernet_BitrateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
            ib_Ethernet_BitrateChangeEvent bitrateChangeEvent);
      
    - Methods

      + old: 

      .. code-block:: c++

        typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterReceiveMessageHandler_t)(
          ib_Ethernet_Controller* controller, 
          void* context,
          ib_Ethernet_ReceiveMessageHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterFrameAckHandler_t)(
          ib_Ethernet_Controller* controller,
          void* context,
          ib_Ethernet_FrameAckHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterStateChangedHandler_t)(
          ib_Ethernet_Controller* controller,
          void* context,
          ib_Ethernet_StateChangedHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterBitRateChangedHandler_t)(
          ib_Ethernet_Controller* controller, 
          void* context,
          ib_Ethernet_BitRateChangedHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_SendFrame_t)(
          ib_Ethernet_Controller* controller,
          ib_Ethernet_Frame* frame,
          void* userContext);

      + new:

      .. code-block:: c++
      
        typedef ib_ReturnCode(*ib_Ethernet_Controller_AddFrameHandler_t)(
          ib_Ethernet_Controller* controller, 
          void* context,
          ib_Ethernet_FrameHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_AddFrameTransmitHandler_t)(
          ib_Ethernet_Controller* controller,
          void* context,
          ib_Ethernet_FrameTransmitHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_AddStateChangeHandler_t)(
          ib_Ethernet_Controller* controller,
          void* context,
          ib_Ethernet_StateChangeHandler_t handler);

        typedef ib_ReturnCode(*ib_Ethernet_Controller_AddBitrateChangeHandler_t)(
          ib_Ethernet_Controller* controller, 
          void* context,
          ib_Ethernet_BitrateChangeHandler_t handler);

Removed
~~~~~~~

- The Lin Controller's ``SendFrame`` and ``SendFrameHeader`` with explicit timestamp are removed.

  - ``IntegrationBus/include/ib/sim/lin/ILinController.hpp``

      + old: 

      .. code-block:: c++

        ILinController::SendFrame(LinFrame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp);
        ILinController::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp);
        
  - ``IntegrationBus/include/ib/capi/Lin.h``

      + old: 

      .. code-block:: c++

        typedef ib_ReturnCode (*ib_Lin_Controller_SendFrameWithTimestamp_t)(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
          ib_Lin_FrameResponseType responseType, ib_NanosecondsTime timestamp);
          
        typedef ib_ReturnCode (*ib_Lin_Controller_SendFrameHeaderWithTimestamp_t)(ib_Lin_Controller* controller, ib_Lin_Id linId,
          ib_NanosecondsTime timestamp);

- In the Cpp-Api, the Can Controller's ``SendMessage`` variant with R-value CanFrame is removed.

  - ``IntegrationBus/include/ib/sim/can/ICanController.hpp``

      + old: 

      .. code-block:: c++

        ICanController::SendMessage(CanMessage&& msg, void* userContext = nullptr) -> CanTxId;

- In the Cpp-Api, the Ethernet Controller's ``SendFrame`` variant with explicit timestamp is removed.

  - ``IntegrationBus/include/ib/sim/eth/IEthController.hpp``

      + old: 

      .. code-block:: c++

        IEthController::SendFrame(EthFrame msg, std::chrono::nanoseconds timestamp) -> EthTxId;

Fixed
~~~~~

- Guard against execution of an already scheduled *SimulationTask* if the state of the simulation is not *Running*.


[3.99.20] - 2022-04-20
--------------------------------

This delivery is the first public delivery since 3.6.1. Since then, development has focused on preparing the public
open source release of the Vector Integration Bus in August 2022. This delivery is not ABI/API/network compatible to
earlier 3.x deliveries. Future 3.99.x deliveries are not expected to be API/ABI/network compatible to each other as we finalize 
the API and network layer for the public open source 4.0 release. These deliveries are intended for evaluation purposes 
and to make the current status of the Vector Integration Bus as transparent as possible.

In this delivery the DataMessage API is stabilized such that it resembles the final API as closely as possible.
See the earlier changelog entries since 3.6.1 for further information considering API changes (especially 3.99.19).
Other parts will still undergo changes for improved consistency within the API.

Major changes since 3.6.1 (see changelog for details):

 - Introduction of optional and distributed participant configuration
 - DataMessage API as replacement for IoControllers and GenericMessage API
 - Remote Procedure Call API
 - Refactoring of CLI of Utilities to be consistent
 - Tracing and Replay currently not functional


Changed
~~~~~~~

- In the C-Api, the Participant's ``ib_Participant_RegisterParticipantStateHandler`` is removed and replaced by
  the more detailed variant ``ib_Participant_RegisterParticipantStatusHandler``.
  
  - IntegrationBus/include/ib/capi/Participant.h
  
      + old: 
      
      .. code-block:: c++

        typedef void (*ib_ParticipantStateHandler_t)(void* context, ib_Participant* participant,
            const char* participantName, ib_ParticipantState state);

        // typedef was missing, API is:
        IntegrationBusAPI ib_ReturnCode ib_Participant_RegisterParticipantStateHandler(ib_Participant* participant,
            void* context, ib_ParticipantStateHandler_t handler);
      
      + new: 
      
      .. code-block:: c++

        typedef struct 
        {
            ib_InterfaceIdentifier interfaceId;
            const char* participantName; //!< Name of the participant.
            ib_ParticipantState participantState; //!< The new state of the participant.
            const char* enterReason; //!< The reason for the participant to enter the new state.
            ib_NanosecondsWallclockTime enterTime; //!< The enter time of the participant.
            ib_NanosecondsWallclockTime refreshTime; //!< The refresh time.
        } ib_ParticipantStatus;

        typedef void (*ib_ParticipantStatusHandler_t)(void* context, ib_Participant* participant,
            const char* participantName, ib_ParticipantStatus status);
            
        typedef ib_ReturnCode (*ib_Participant_RegisterParticipantStatusHandler_t)(ib_Participant* participant, void* context,
            ib_ParticipantStatusHandler_t handler);
            

Removed
~~~~~~~

- The SystemMonitor's ``RegisterParticipantStateHandler`` is removed, its functionality is already covered by
  ``RegisterParticipantStatusHandler``.

  - IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp 

      + old: 
      
      .. code-block:: c++

        ISystemMonitor::RegisterParticipantStateHandler(ParticipantStateHandlerT handler);

Fixed
~~~~~~~

- Fixed bug of no data transmission related to delayed DefaultDataMessageHandler registration


.. raw:: html

  <details>
  <summary>Complete list of changes to the C++ API (click to expand)</summary>

.. code-block:: c++

  --- a/IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp

   class ISystemMonitor

  -    using ParticipantStateHandlerT = std::function<void(ParticipantState)>;
  -    virtual void RegisterParticipantStateHandler(ParticipantStateHandlerT handler) = 0;

.. raw:: html

  </details>


.. raw:: html

  <details>
  <summary>Complete list of changes to the C API (click to expand)</summary>

.. code-block:: 

  --- IntegrationBus/include/ib/capi/InterfaceIdentifiers.h

  +#define ib_InterfaceIdentifier_ParticipantStatus           ((ib_InterfaceIdentifier)7001001)

  --- a/IntegrationBus/include/ib/capi/Participant.h

  +typedef uint64_t ib_NanosecondsWallclockTime; //!< Wall clock time since epoch

  +typedef struct
  +{
  +    ib_InterfaceIdentifier interfaceId;
  +    const char* participantName; //!< Name of the participant.
  +    ib_ParticipantState participantState; //!< The new state of the participant.
  +    const char* enterReason; //!< The reason for the participant to enter the new state.
  +    ib_NanosecondsWallclockTime enterTime; //!< The enter time of the participant.
  +    ib_NanosecondsWallclockTime refreshTime; //!< The refresh time.
  +} ib_ParticipantStatus;

  -typedef void (*ib_ParticipantStateHandler_t)(void* context, ib_Participant* participant, const char* participantName, ib_ParticipantState state);

  +typedef void (*ib_ParticipantStatusHandler_t)(void* context, ib_Participant* participant, const char* participantName, ib_ParticipantStatus status);

  -IntegrationBusAPI ib_ReturnCode ib_Participant_RegisterParticipantStateHandler(ib_Participant* participant, void* context, ib_ParticipantStateHandler_t handler);
  +IntegrationBusAPI ib_ReturnCode ib_Participant_RegisterParticipantStatusHandler(ib_Participant* participant, void* context, ib_ParticipantStatusHandler_t handler);

  +typedef ib_ReturnCode (*ib_Participant_RegisterParticipantStatusHandler_t)(ib_Participant* participant, void* context, ib_ParticipantStatusHandler_t handler);

.. raw:: html

  </details>


[3.99.19] - 2022-04-19
--------------------------------

In this delivery we stabilized the DataMessage API.

Added
~~~~~

- Participant methods to create DataPublisher, DataSubscriber, RpcClient and RpcServer now have an simplified
  overload with the controller name ('canonicalName') as single argument. In this variant, the controller name is used as topic/rpcChannel.
  The C-Api doesn't provide these simplified methods.

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    + new:

    .. code-block:: c++
      
      IParticipant::CreateDataPublisher(const string& canonicalName) -> ...;
      IParticipant::CreateDataSubscriber(const string& canonicalName) -> ...;
      IParticipant::CreateRpcClient(const string& canonicalName) -> ...;
      IParticipant::CreateRpcServer(const string& canonicalName) -> ...;

Removed
~~~~~~~

- The typedef ``ib::mw::sync::IParticipantController::TaskHandleT`` was removed due to not being used anywhere.

- The header ``IntegrationBus/include/ib/mw/sync/ISyncMaster.hpp`` was removed due to not being used anywhere.

- Replaced the single-member struct ``DataExchangeFormat`` with its sole member, the media type string.
  The following data type and some associated free functions were removed:

  - ``IntegrationBus/include/ib/sim/data/DataMessageDatatypes.hpp``

    .. code-block:: c++

      struct ib::sim::data::DataExchangeFormat;
      bool operator ==(const DataExchangeFormat &, const DataExchangeFormat &);

  - ``IntegrationBus/include/ib/sim/data/string_utils.hpp``

    .. code-block:: c++

      ib::sim::data::to_string(const DataExchangeFormat&) -> std::string;
      ib::sim::data::operator<<(std::ostream& out, const DataExchangeFormat& dataExchangeFormat) -> std::ostream&;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    .. code-block:: c++

      typedef struct { ... } ib_Data_ExchangeFormat;

Changed
~~~~~~~

- Updated documentation of :doc:`./usage/demos`, :doc:`./usage/utilities` and :doc:`./configuration/configuration`


- Participant (formerly 'ComAdapter') methods to create DataPublisher, DataSubscriber, RpcClient and RpcServer now have an additional 
  argument for the controller name. The controller name is used to to reference the controller in the configuration file 
  (formerly, 'topic'/'functionName' was used).

  - ``IntegrationBus/include/ib/mw/IComAdapter.hpp``

    + new:

    .. code-block:: c++
      
      IComAdapter::CreateDataPublisher(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateDataSubscriber(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateRpcClient(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateRpcServer(const string& canonicalName, ...) -> ...;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + new:

    .. code-block:: c++
  
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(ib_Data_Publisher** outPublisher, ib_Participant* participant,
                                                        const char* controllerName, ...);

      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(ib_Data_Subscriber** outSubscriber, ib_Participant* participant,
                                                              const char* controllerName, ...);

- Renamed Public-API for DataSubscriber:

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    + old: 
    
    .. code-block:: c++
    
      IDataSubscriber::SetDefaultReceiveMessageHandler(...);
      IDataSubscriber::RegisterSpecificDataHandler(...);

    + new:

    .. code-block:: c++

      IDataSubscriber::SetDefaultDataMessageHandler(...);
      IDataSubscriber::AddExplicitDataMessageHandler(...);

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + old:

    .. code-block:: c++
    
      typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultReceiveDataHandler_t)(...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_RegisterSpecificDataHandler_t)(...);

    + new:

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultDataMessageHandler_t)(...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_AddExplicitDataMessageHandler_t)(...);

- Renamed/wrapped structs and using statements in PubSub-context for the C/Cpp-API:

    + old: 
    
    .. code-block:: c++

      // Cpp-API
      using DataHandlerT =
      std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data)>;
      
      using NewDataSourceHandlerT = std::function<void(ib::sim::data::IDataSubscriber* subscriber,
                                                      const std::string& topic, const std::string& mediaType,
                                                      const std::map<std::string, std::string>& labels)>;
      
      // C-API
      typedef void (*ib_Data_Handler_t)(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data);
      
      typedef void (*ib_Data_NewDataSourceHandler_t)(void* context, ib_Data_Subscriber* subscriber, const char* topic,
                                                      const char* mediaType, const ib_KeyValueList* labels);

    + new:

    The new DataMessageEvent now contains a timestamp with the send time set by the DataPublisher. 
    Formerly, the reception callback only contained the raw data. The information about a new DataPublisher in the 
    NewDataPublisherHandlerT now is bundeled in a stuct called 'NewDataPublisherEvent' also containing a reception
    timestamp.

      .. code-block:: c++

        // Cpp-API
        using DataMessageHandlerT =
        std::function<void(ib::sim::data::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)>;
        
        using NewDataPublisherHandlerT =
        std::function<void(ib::sim::data::IDataSubscriber* subscriber, const NewDataPublisherEvent& newDataPublisherEvent)>;
      
        // C-API
        typedef void (*ib_Data_DataMessageHandler_t)(void* context, ib_Data_Subscriber* subscriber, 
                                                    const ib_Data_DataMessageEvent* dataMessageEvent);
      
        typedef void (*ib_Data_NewDataPublisherHandler_t)(void* context, ib_Data_Subscriber* subscriber,
                                                          const ib_Data_NewDataPublisherEvent* newDataPublisherEvent);

- The header ``EndpointAddress.hpp``, ``IReplay.hpp``, ``ITraceMessageSink.hpp``, ``ITraceMessageSource.hpp`` and
  ``TraceMessage.hpp`` are now internal headers.
  The typedef ``ib::mw::ParticipantId`` which used to be defined here was moved into its own public header file.

  - ``IntegrationBus/include/ib/mw/ParticipantId.hpp``

    + new:

    .. code-block:: c++

      using ParticipantId = ...;

  - ``IntegrationBus/include/ib/mw/sync/SyncDatatypes.hpp``

    + old:

    .. code-block:: c++

      #include "ib/mw/EndpointAddress.hpp"

    + new:

    .. code-block:: c++

      #include "ib/mw/ParticipantId.hpp"

- Replaced the single-member struct ``DataExchangeFormat`` with its sole member, the media type string.
  The following method and function type signatures have changed:


    + old:

    .. code-block:: c++

      IComAdapter::CreateDataPublisher(..., const DataExchangeFormat&, ...) -> ...;
      IComAdapter::CreateDataSubscriber(..., const DataExchangeFormat&, ...) -> ...;

    + new:

    .. code-block:: c++

      IComAdapter::CreateDataPublisher(..., const std::string& ...) -> ...;
      IComAdapter::CreateDataSubscriber(..., const std::string& ...) -> ...;

  - ``IntegrationBus/include/ib/sim/data/DataMessageDatatypes.hpp``

    + old:

    .. code-block:: c++

      NewDataSourceHandlerT = std::function<void(..., const DataExchangeFormat&, ...)>;

    + new:

    .. code-block:: c++

      NewDataSourceHandlerT = std::function<void(..., const std::string&, ...)>;

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    + old:

    .. code-block:: c++

      IDataSubscriber::RegisterSpecificDataHandler(const DataExchangeFormat&, ...) -> ...;

    + new:

    .. code-block:: c++

      IDataSubscriber::RegisterSpecificDataHandler(const std::string&, ...) -> ...;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + old:

    .. code-block:: c++

      typedef void (*ib_Data_NewDataSourceHandler_t)(..., const ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_Create(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);

    + new:

    .. code-block:: c++

      typedef void (*ib_Data_NewDataSourceHandler_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(..., const char* mediaType, ...);
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_Create(..., const char* mediaType, ...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(..., const char* mediaType, ...);

- Renamed ``ib::mw::IComAdapter`` to ``ib::mw::IParticipant``, and 
  renamed ``IntegrationBus/include/ib/mw/IComAdapter.hpp`` to ``IntegrationBus/include/ib/mw/IParticipant.hpp``.
  Further renamed method ``ib::CreateSimulationParticipant(...)`` to ``ib::CreateParticipant(...)``:

  - ``IntegrationBus/include/ib/IntegrationBus.hpp``
  
    + old:

    .. code-block:: c++
      
      IntegrationBusAPI auto CreateSimulationParticipant(
          std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig, const std::string& participantName,
          const uint32_t domainId, bool isSynchronized) -> std::unique_ptr<mw::IComAdapter>;
    
    + new:

    .. code-block:: c++
      
      IntegrationBusAPI auto CreateParticipant(
          std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig, const std::string& participantName,
          const uint32_t domainId, bool isSynchronized) -> std::unique_ptr<mw::IParticipant>;

    .. Admonition:: Note
       
       The isSynchronized parameter is temporary and will be removed in the future.

.. raw:: html

  <details>
  <summary>Complete list of changes to the C++ API (click to expand)</summary>

.. code-block:: c++

  --- IntegrationBus/include/ib/IntegrationBus.hpp

  -CreateSimulationParticipant(...) -> std::unique_ptr<mw::IComAdapter>;
  +CreateParticipant(...) -> std::unique_ptr<mw::IParticipant>;

  --- IntegrationBus/include/ib/mw/IParticipant.hpp

  rename from IntegrationBus/include/ib/mw/IComAdapter.hpp
  rename to IntegrationBus/include/ib/mw/IParticipant.hpp

  -class IComAdapter
  +class IParticipant

  -    virtual ~IComAdapter() = default;
  +    virtual ~IParticipant() = default;

  +    virtual auto CreateDataPublisher(const std::string& canonicalName) -> sim::data::IDataPublisher* = 0;
  +    virtual auto CreateDataSubscriber(const std::string& canonicalName) -> sim::data::IDataSubscriber* = 0;
  +    virtual auto CreateRpcClient(const std::string& canonicalName) -> sim::rpc::IRpcClient* = 0;
  +    virtual auto CreateRpcServer(const std::string& canonicalName) -> sim::rpc::IRpcServer* = 0;

  -    virtual auto CreateDataPublisher(const std::string& topic, const sim::data::DataExchangeFormat& dataExchangeFormat, ...);
  +    virtual auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic, const std::string& mediaType, ...);

  -    virtual auto CreateDataSubscriber(
  -            const std::string& topic,
  -            const sim::data::DataExchangeFormat& dataExchangeFormat,
  -            ...,
  -            sim::data::DataHandlerT defaultDataHandler,
  -            sim::data::NewDataSourceHandlerT newDataSourceHandler = nullptr) -> ...;
  +    virtual auto CreateDataSubscriber(
  +            const std::string& canonicalName,
  +            const std::string& topic, const std::string& mediaType,
  +            ...,
  +            sim::data::DataMessageHandlerT defaultDataMessageHandler,
  +            sim::data::NewDataPublisherHandlerT newDataPublisherHandler = nullptr) -> ...;

  -    virtual auto CreateRpcClient(const std::string& functionName, ...) -> ...;
  +    virtual auto CreateRpcClient(const std::string& canonicalName, const std::string& channel, ...) -> ...;

  -    virtual auto CreateRpcServer(const std::string& functionName, ...) -> ...;
  +    virtual auto CreateRpcServer(const std::string& canonicalName, const std::string& channel, ...) -> ...;

  -    virtual void DiscoverRpcServers(const std::string& functionName, ...);
  +    virtual void DiscoverRpcServers(const std::string& rpcChannel, ...);

  --- IntegrationBus/include/ib/mw/ParticipantId.hpp

  +using ParticipantId = uint64_t;

  --- IntegrationBus/include/ib/mw/sync/IParticipantController.hpp

  -using TaskHandleT = void*;

  --- IntegrationBus/include/ib/mw/sync/ISyncMaster.hpp

  removed IntegrationBus/include/ib/mw/sync/ISyncMaster.hpp

  --- IntegrationBus/include/ib/mw/sync/ISystemMonitor.hpp

  -    virtual auto ParticipantStatus(const std::string& participantId) const -> const sync::ParticipantStatus& = 0;
  +    virtual auto ParticipantStatus(const std::string& participantName) const -> const sync::ParticipantStatus& = 0;

  --- IntegrationBus/include/ib/sim/data/DataMessageDatatypes.hpp

  -struct DataExchangeFormat {
  -    std::string mediaType;
  -};

  -inline bool operator==(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs)

  -using DataHandlerT =
  -    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data)>;
  +using DataMessageHandlerT =
  +    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)>;

  -struct DataMessage {
  -    std::vector<uint8_t> data;
  -};
  +struct DataMessageEvent
  +{
  +    std::chrono::nanoseconds timestamp;
  +    std::vector<uint8_t> data;
  +};

  +struct NewDataPublisherEvent
  +{
  +    std::chrono::nanoseconds timestamp;
  +    std::string topic;
  +    std::string mediaType;
  +    std::map<std::string, std::string> labels;
  +};

  -using NewDataSourceHandlerT = std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::string& topic,
  -                                                 const ib::sim::data::DataExchangeFormat& dataExchangeFormat,
  -                                                 const std::map<std::string, std::string>& labels)>;
  +using NewDataPublisherHandlerT =
  +    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const NewDataPublisherEvent& newDataPublisherEvent)>;

  --- IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp

  -    virtual void SetDefaultReceiveMessageHandler(DataHandlerT callback) = 0;
  +    virtual void SetDefaultDataMessageHandler(DataMessageHandlerT callback) = 0;

  -    virtual void RegisterSpecificDataHandler(const DataExchangeFormat& dataExchangeFormat,
  -                                             const std::map<std::string, std::string>& labels,
  -                                             DataHandlerT callback) = 0;
  +    virtual void AddExplicitDataMessageHandler(DataMessageHandlerT callback,
  +                                               const std::string& mediaType,
  +                                               const std::map<std::string, std::string>& labels) = 0;

  --- IntegrationBus/include/ib/sim/data/fwd_decl.hpp

  -struct DataMessage;
  -struct DataExchangeFormat;
  +struct DataMessageEvent;

  --- IntegrationBus/include/ib/sim/data/string_utils.hpp

  -inline std::string to_string(const DataExchangeFormat& dataExchangeFormat);
  -inline std::ostream& operator<<(std::ostream& out, const DataExchangeFormat& dataExchangeFormat);

  -inline std::string to_string(const DataMessage& msg);
  +inline std::string to_string(const DataMessageEvent& msg);

  -inline std::ostream& operator<<(std::ostream& out, const DataMessage& msg);
  +inline std::ostream& operator<<(std::ostream& out, const DataMessageEvent& msg);

  -std::string to_string(const DataMessage& msg)
  +std::string to_string(const DataMessageEvent& msg)

  -std::ostream& operator<<(std::ostream& out, const DataMessage& msg)
  +std::ostream& operator<<(std::ostream& out, const DataMessageEvent& msg)

  -std::string to_string(const DataExchangeFormat& dataExchangeFormat)
  -std::ostream& operator<<(std::ostream& out, const DataExchangeFormat& dataExchangeFormat)

  --- IntegrationBus/include/ib/sim/rpc/RpcDatatypes.hpp

   struct RpcDiscoveryResult
   {
  -    std::string functionName;
  +    std::string rpcChannel;
       ...
   };

  --- IntegrationBus/include/ib/sim/rpc/string_utils.hpp

  -inline std::string   to_string(const RpcExchangeFormat& dataExchangeFormat);
  +inline std::string   to_string(const RpcExchangeFormat& rpcExchangeFormat);

  -inline std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& dataExchangeFormat);
  +inline std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& rpcExchangeFormat);

.. raw:: html

  </details>

.. raw:: html

  <details>
  <summary>Complete List of changes to the C API (click to expand)</summary>

.. code-block::

  --- IntegrationBus/include/ib/capi/Types.h

  -typedef struct ib_SimulationParticipant ib_SimulationParticipant;
  +typedef struct ib_Participant ib_Participant;

  --- IntegrationBus/include/ib/capi/IntegrationBus.h

  -typedef ib_ReturnCode(*ib_SimulationParticipant_GetLogger_t)(..., ib_SimulationParticipant* participant);
  +typedef ib_ReturnCode(*ib_Participant_GetLogger_t)(..., ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_GetLogger(..., ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_GetLogger(..., ib_Participant* participant);

  --- IntegrationBus/include/ib/capi/InterfaceIdentifiers.h

  -#define ib_InterfaceIdentifier_DataExchangeFormat          ((ib_InterfaceIdentifier)5001001)
  +#define ib_InterfaceIdentifier_DataMessageEvent            ((ib_InterfaceIdentifier)5001001)
  +#define ib_InterfaceIdentifier_NewDataPublisherEvent       ((ib_InterfaceIdentifier)5001002)

  --- IntegrationBus/include/ib/capi/Participant.h

  rename from IntegrationBus/include/ib/capi/SimulationParticipant.h
  rename to IntegrationBus/include/ib/capi/Participant.h

  -ib_ReturnCode ib_SimulationParticipant_Create(ib_SimulationParticipant** outParticipant, ...);
  +ib_ReturnCode ib_Participant_Create(ib_Participant** outParticipant, ...);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_Create_t)(ib_SimulationParticipant** outParticipant, ...);
  +typedef ib_ReturnCode (*ib_Participant_Create_t)(ib_Participant** outParticipant, ...);

  -ib_ReturnCode ib_SimulationParticipant_Destroy(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_Destroy(ib_Participant* participant);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_Destroy_t)(ib_SimulationParticipant* participant);
  +typedef ib_ReturnCode (*ib_Participant_Destroy_t)(ib_Participant* participant);

  -typedef void (*ib_ParticipantInitHandler_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef void (*ib_ParticipantInitHandler_t)(.., ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_SetInitHandler(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetInitHandler(ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_SetInitHandler_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode(*ib_Participant_SetInitHandler_t)(ib_Participant* participant, ...);

  -typedef void (*ib_ParticipantStopHandler_t)(..., ib_SimulationParticipant* participant);
  +typedef void (*ib_ParticipantStopHandler_t)(..., ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_SetStopHandler(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetStopHandler(ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_SetStopHandler_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode(*ib_Participant_SetStopHandler_t)(ib_Participant* participant, ...);

  -typedef void (*ib_ParticipantShutdownHandler_t)(..., ib_SimulationParticipant* participant);
  +typedef void (*ib_ParticipantShutdownHandler_t)(..., ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_SetShutdownHandler(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetShutdownHandler(ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_SetShutdownHandler_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode(*ib_Participant_SetShutdownHandler_t)(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_Run(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_Run(ib_Participant* participant, ...);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_Run_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode (*ib_Participant_Run_t)(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_RunAsync(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_RunAsync(ib_Participant* participant);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_RunAsync_t)(ib_SimulationParticipant* participant);
  +typedef ib_ReturnCode (*ib_Participant_RunAsync_t)(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_WaitForRunAsyncToComplete(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_WaitForRunAsyncToComplete(ib_Participant* participant, ...);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_WaitForRunAsyncToComplete_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode (*ib_Participant_WaitForRunAsyncToComplete_t)(ib_Participant* participant, ...);

  -typedef ib_ReturnCode (*ib_SimulationParticipant_SetPeriod_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode (*ib_Participant_SetPeriod_t)(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_SetPeriod(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetPeriod(ib_Participant* participant, ...);

  -typedef void (*ib_ParticipantSimulationTaskHandler_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef void (*ib_ParticipantSimulationTaskHandler_t)(..., ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_SetSimulationTask(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetSimulationTask(ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_SetSimulationTask_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode(*ib_Participant_SetSimulationTask_t)(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_SetSimulationTaskAsync(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetSimulationTaskAsync(ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_SetSimulationTaskNonBlocking_t)(ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode(*ib_Participant_SetSimulationTaskNonBlocking_t)(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_CompleteSimulationTask(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_CompleteSimulationTask(ib_Participant* participant);

  -typedef ib_ReturnCode(*ib_SimulationParticipant_CompleteSimulationTask_t)(ib_SimulationParticipant* participant);
  +typedef ib_ReturnCode(*ib_Participant_CompleteSimulationTask_t)(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_Initialize(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_Initialize(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_Reinitialize(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_Reinitialize(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_RunSimulation(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_RunSimulation(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_StopSimulation(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_StopSimulation(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_Pause(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_Pause(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_Continue(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_Continue(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_Shutdown(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_Shutdown(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_PrepareColdswap(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_PrepareColdswap(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_ExecuteColdswap(ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_ExecuteColdswap(ib_Participant* participant);

  -ib_ReturnCode ib_SimulationParticipant_GetParticipantState(..., ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_GetParticipantState(..., ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_GetSystemState(..., ib_SimulationParticipant* participant);
  +ib_ReturnCode ib_Participant_GetSystemState(..., ib_Participant* participant);

  -typedef void (*ib_SystemStateHandler_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef void (*ib_SystemStateHandler_t)(..., ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_RegisterSystemStateHandler(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_RegisterSystemStateHandler(ib_Participant* participant, ...);

  -typedef void (*ib_ParticipantStateHandler_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef void (*ib_ParticipantStateHandler_t)(..., ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_RegisterParticipantStateHandler(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_RegisterParticipantStateHandler(ib_Participant* participant, ...);

  -ib_ReturnCode ib_SimulationParticipant_SetRequiredParticipants(ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Participant_SetRequiredParticipants(ib_Participant* participant, ...);

  --- IntegrationBus/include/ib/capi/Can.h

  -ib_ReturnCode ib_Can_Controller_Create(..., ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Can_Controller_Create(..., ib_Participant* participant, ...);

  -typedef ib_ReturnCode (*ib_Can_Controller_Create_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode (*ib_Can_Controller_Create_t)(..., ib_Participant* participant, ...);

  --- IntegrationBus/include/ib/capi/Ethernet.h

  -ib_ReturnCode ib_Ethernet_Controller_Create(..., ib_SimulationParticipant* participant, ...);
  +ib_ReturnCode ib_Ethernet_Controller_Create(..., ib_Participant* participant, ...);

  -typedef ib_ReturnCode(*ib_Ethernet_Controller_Create_t)(..., ib_SimulationParticipant* participant, ...);

  --- IntegrationBus/include/ib/capi/FlexRay.h
  -ib_ReturnCode ib_FlexRay_Controller_Create(..., ib_SimulationParticipant* participant, ...);
  -ib_ReturnCode ib_FlexRay_Controller_Create(..., ib_Participant* participant, ...);

  -typedef ib_ReturnCode (*ib_FlexRay_Controller_Create_t)(..., ib_SimulationParticipant* participant, ...);
  +typedef ib_ReturnCode (*ib_FlexRay_Controller_Create_t)(..., ib_Participant* participant, ...);

  --- IntegrationBus/include/ib/capi/Lin.h

  -ib_ReturnCode ib_Lin_Controller_Create(..., ib_SimulationParticipant *participant, ...);
  +ib_ReturnCode ib_Lin_Controller_Create(..., ib_Participant *participant, ...);

  -typedef ib_ReturnCode (*ib_Lin_Controller_Create_t)(..., ib_SimulationParticipant* participant, ...);
  -typedef ib_ReturnCode (*ib_Lin_Controller_Create_t)(..., ib_Participant* participant, ...);

  --- IntegrationBus/include/ib/capi/Rpc.h

   typedef struct ib_Rpc_DiscoveryResult
   {
       ...
  -    const char* functionName;
  +    const char* rpcChannel;
       ...
   } ib_Rpc_DiscoveryResult;

  -ib_ReturnCode ib_Rpc_Server_Create(..., ib_SimulationParticipant* participant, const char* functionName, ...)
  +ib_ReturnCode ib_Rpc_Server_Create(..., ib_Participant* participant, const char* controllerName, const char* rpcChannel, ...);

  -typedef ib_ReturnCode (*ib_Rpc_Server_Create_t)(..., ib_SimulationParticipant* participant, const char* functionName, ...);
  +typedef ib_ReturnCode (*ib_Rpc_Server_Create_t)(..., ib_Participant* participant, const char* controllerName, const char* rpcChannel, ...);

  -ib_ReturnCode ib_Rpc_Client_Create(..., ib_SimulationParticipant* participant, const char* functionName, ...);
  +ib_ReturnCode ib_Rpc_Client_Create(..., ib_Participant* participant, const char* controllerName, const char* rpcChannel, ...);

  -typedef ib_ReturnCode (*ib_Rpc_Client_Create_t)(..., ib_SimulationParticipant* participant, const char* functionName, ...);
  +typedef ib_ReturnCode (*ib_Rpc_Client_Create_t)(..., ib_Participant* participant, const char* controllerName, const char* rpcChannel, ...);

  -ib_ReturnCode ib_Rpc_DiscoverServers(ib_SimulationParticipant* participant, const char* functionName, ...);
  +ib_ReturnCode ib_Rpc_DiscoverServers(ib_Participant* participant, const char* rpcChannel, ...);

  -typedef ib_ReturnCode(*ib_Rpc_DiscoverServers_t)(ib_SimulationParticipant* participant, const char* functionName, ...);
  +typedef ib_ReturnCode(*ib_Rpc_DiscoverServers_t)(ib_Participant* participant, const char* rpcChannel, ...);

  --- IntegrationBus/include/ib/capi/DataPubSub.h

  -typedef struct { ... } ib_Data_ExchangeFormat;

  +typedef struct
  +{
  +    ib_InterfaceIdentifier interfaceId;
  +    ib_NanosecondsTime timestamp;
  +    ib_ByteVector data;
  +} ib_Data_DataMessageEvent;

  +typedef struct
  +{
  +    ib_InterfaceIdentifier interfaceId;
  +    ib_NanosecondsTime timestamp;
  +    const char* topic;
  +    const char* mediaType;
  +    ib_KeyValueList* labels;
  +} ib_Data_NewDataPublisherEvent;

  -typedef void (*ib_Data_Handler_t)(..., const ib_ByteVector* data);
  +typedef void (*ib_Data_DataMessageHandler_t)(..., const ib_Data_DataMessageEvent* dataMessageEvent);

  -typedef void (*ib_Data_NewDataSourceHandler_t)(..., const char* topic, const ib_Data_ExchangeFormat* dataExchangeFormat, const ib_KeyValueList* labels);
  +typedef void (*ib_Data_NewDataPublisherHandler_t)(..., const ib_Data_NewDataPublisherEvent* newDataPublisherEvent);

  -ib_ReturnCode ib_Data_Publisher_Create(..., ib_SimulationParticipant* participant, const char* topic, ib_Data_ExchangeFormat* dataExchangeFormat, ...);
  +ib_ReturnCode ib_Data_Publisher_Create(..., ib_Participant* participant, const char* controllerName, const char* topic, const char* mediaType, ...);

  -typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., ib_SimulationParticipant* participant, const char* topic, ib_Data_ExchangeFormat* dataExchangeFormat, ...);
  +typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., ib_Participant* participant, const char* controllerName, const char* topic, const char* mediaType, ...);

  -ib_ReturnCode ib_Data_Subscriber_Create(
  -        ...,
  -        ib_SimulationParticipant* participant,
  -        const char* topic,
  -        ib_Data_ExchangeFormat* dataExchangeFormat,
  -        ...,
  -        ib_Data_Handler_t defaultDataHandler,
  -        ...,
  -        ib_Data_NewDataSourceHandler_t newDataSourceHandler);
  +ib_ReturnCode ib_Data_Subscriber_Create(
  +        ...,
  +        ib_Participant* participant,
  +        const char* controllerName,
  +        const char* topic,
  +        const char* mediaType,
  +        ...,
  +        ib_Data_DataMessageHandler_t defaultDataHandler,
  +        ...,
  +        ib_Data_NewDataPublisherHandler_t newDataSourceHandler);

  -typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(
  -        ...,
  -        ib_SimulationParticipant* participant,
  -        const char* topic,
  -        ib_Data_ExchangeFormat* dataExchangeFormat,
  -        ...,
  -        ib_Data_Handler_t defaultDataHandler,
  -        ...,
  -        ib_Data_NewDataSourceHandler_t newDataSourceHandler);
  +typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(
  +        ...,
  +        ib_Participant* participant,
  +        const char* controllerName,
  +        const char* topic,
  +        const char* mediaType,
  +        ...,
  +        ib_Data_DataMessageHandler_t defaultDataHandler,
  +        ...,
  +        ib_Data_NewDataPublisherHandler_t newDataSourceHandler);

  -typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultReceiveDataHandler_t)(
  -        ...,
  -        ib_Data_Handler_t dataHandler);
  +typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultDataMessageHandler_t)(
  +        ...,
  +        ib_Data_DataMessageHandler_t dataHandler);

  -ib_ReturnCode ib_Data_Subscriber_SetDefaultReceiveDataHandler(
  -        ...,
  -        ib_Data_Handler_t dataHandler);
  +ib_ReturnCode ib_Data_Subscriber_SetDefaultDataMessageHandler(
  +        ...,
  +        ib_Data_DataMessageHandler_t dataHandler);

  -ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(
  -        ib_Data_Subscriber* self,
  -        ib_Data_ExchangeFormat* dataExchangeFormat,
  -        const ib_KeyValueList* labels,
  -        void* context,
  -        ib_Data_Handler_t dataHandler);
  +ib_ReturnCode ib_Data_Subscriber_AddExplicitDataMessageHandler(
  +        ib_Data_Subscriber* self,
  +        void* context,
  +        ib_Data_DataMessageHandler_t dataHandler,
  +        const char* mediaType,
  +        const ib_KeyValueList* labels);

  -typedef ib_ReturnCode (*ib_Data_Subscriber_RegisterSpecificDataHandler_t)(
  -        ib_Data_Subscriber* self,
  -        ib_Data_ExchangeFormat* dataExchangeFormat,
  -        const ib_KeyValueList* labels,
  -        void* context,
  -        ib_Data_Handler_t dataHandler);
  +typedef ib_ReturnCode (*ib_Data_Subscriber_AddExplicitDataMessageHandler_t)(
  +        ib_Data_Subscriber* self,
  +        void* context,
  +        ib_Data_DataMessageHandler_t dataHandler,
  +        const char* mediaType,
  +        const ib_KeyValueList* labels);

.. raw:: html

  </details>


[3.7.18] - 2022-04-05
--------------------------------

Added
~~~~~
- Support for MinGW builds was added.
  The CI builds use the native MinGW builds from winlibs, but a
  cross-compile CMake toolchain file is provided in `IntegrationBus/cmake/mingw-w64-toolchain.cmake`.
  Warnings are not treated as errors for this compilation target, due to some known
  issues in the MinGW-w64 toolchain.

  MinGW libraries are not distributed as part of VIB packages.
  The user must ensure that the following libraries are in PATH:

    .. code-block:: sh

        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
   
Fixed
~~~~~
- Fixed typo in IbRegistry command help output.
- Fixed typo in IbSystemMonitor command help output.
- The IbRegistry hostname defined in the configuration is now resolved and
  used to create listening sockets.
  IPv6 addresses are not supported, yet.
- The `lin::LinControllerConfig` data type now uses a message history of size 1.
  If the value was set by a `LinController::Init` call, it will be
  retained and automatically transmitted to newly connecting participants.
  
Changed
~~~~~~~
- Demo adaptions
  - Ethernet Demo only uses Set/GetRawFrame calls
  - CAN & Ethernet demo can now run as asynchronous participants (add `--async` as command line argument)

- Updated documentation of network simulator and :doc:`./simulation/simulation`


[3.7.13] - 2022-03-24
--------------------------------
This is an internal build.

Changed
~~~~~~~
- Added a new command line parser for all utilities, to have a consistent user experience.
  Long and short options are supported, invoke the utilities with `--help` to
  see a list of supported options:

    + old: 

    .. code-block:: sh

       IbRegistry Configuration.json 42

    + new: 

    .. code-block:: sh

       IbRegistry --domain 42 Configuration.json
       #the following lists all supported options:
       IbRegistry --help

    + old: 

    .. code-block:: sh

       IbSystemController  42 participant1 participant2 ...

    + new: 

    .. code-block:: sh

       IbSystemController  --domain 42 participant1 participant2 ...

  The VIBE-NetworkSimulator also supports this new command line interface.

Fixed:
~~~~~~
- The IntegrationBus shared library does not log to stdout anymore.
  The internal logger is now used when loading extensions or creating named pipes.
- CAN: send a valid FD frame in CAN Demo by setting hte FD format indicator.


[3.7.8] - 2022-03-09
--------------------------------
This is an internal build.

Removed
~~~~~~~

- Removed deprecated ISystemController functions `Initialize()` and `Reinitialize()` 
  which took a numeric participant id as argument.
  Use participant names instead.

    + old:

    .. code-block:: c++
        
      void ib::mw::sync::ISystemController::Initialize(ParticipantId) const;
      void ib::mw::sync::ISystemController::Reinitialize(ParticipantId participantId) const;

    + new:

    .. code-block:: c++
       
       void ib::mw::sync::ISystemController::Initialize(const std::string& participantName) const;
       void ib::mw::sync::ISystemController::Reinitialize(const std::string& participantName) const;


- Removed the ConfigBuilder from the public API in the `include/ib` directory.
  The following headers were removed:
    
  .. code-block:: sh

    include/ib/cfg/ConfigBuilder.hpp
    include/ib/cfg/ControllerBuilder.hpp
    include/ib/cfg/DataPortBuilder.hpp
    include/ib/cfg/ExtensionConfigBuilder.hpp
    include/ib/cfg/GenericPortBuilder.hpp
    include/ib/cfg/LinkBuilder.hpp
    include/ib/cfg/LoggerBuilder.hpp
    include/ib/cfg/NetworkSimulatorBuilder.hpp
    include/ib/cfg/ParentBuilder.hpp
    include/ib/cfg/ParticipantBuilder.hpp
    include/ib/cfg/ParticipantBuilder_detail.hpp
    include/ib/cfg/ReplayBuilder.hpp
    include/ib/cfg/RpcPortBuilder.hpp
    include/ib/cfg/SimulationSetupBuilder.hpp
    include/ib/cfg/SinkBuilder.hpp
    include/ib/cfg/SwitchBuilder.hpp
    include/ib/cfg/SwitchPortBuilder.hpp
    include/ib/cfg/TimeSyncBuilder.hpp
    include/ib/cfg/TraceSinkBuilder.hpp
    include/ib/cfg/TraceSourceBuilder.hpp
    include/ib/cfg/VAsioConfigBuilder.hpp
    include/ib/cfg/VAsioRegistryBuilder.hpp

- Removed all Configuration headers from `include/ib`. The configuration structure is no longer public.
  Use the opaque :cpp:class:`ib::cfg::IParticipantConfiguration` instead.
  The following headers were removed:

  .. code-block:: sh

     IntegrationBus/include/ib/cfg/OptionalCfg.hpp
     IntegrationBus/include/ib/cfg/all.hpp
     IntegrationBus/include/ib/cfg/fwd_decl.hpp
     IntegrationBus/include/ib/cfg/string_utils.hpp

Changed
~~~~~~~
- Adapted the demos to the new configuration file format.
   - Removed all old configuration files.
   - New configuration files only define a logger.
   - Bus system demos also provide an additional configuration file for VIBE-NetSim.
- Network names on controllers are optional.

Fixed:
~~~~~~
- Reduced a lot of compiler warnings. On CI builds, warnings are treated as errors.
- FlexRay: FlexRayController.Configure(...): values provided in the configuration file were not correctly overriding user-defined values.
- All Bus Systems: Fixed bugs related to configured controllers that had no defined network.
- CAN-Controller: Fixed a bug that prevented users from creating configured CAN controllers if the network was configured and the provided network name did not match the configured one.
- Fixed deadlock in destructor when :cpp:func:`SetSimulationTaskAsync()<ib::mw::sync::IParticipantController::SetSimulationTaskAsync()>`
  is used.
- Fixed LinkUp/LinkDown transmission in VIBE-NetworkSimulator.
- Fixed ethernet frame transmission with multiple switches in VIBE-NetworkSimulator.


[3.7.2] - unreleased
--------------------------------

Fixed
~~~~~
- Controller pointer in FlexRay Controller Wakeup handler

[3.7.1] - unreleased
--------------------------------

Fixed
~~~~~
- New Configuration format Network name override for Can and Ethernet


[3.7.0] - unreleased
--------------------------------

Changed
~~~~~~~

.. admonition:: Note: the IntegrationBus configuration and command line usage changed!

   The old simulation setup based configuration mechanism was replaced by a 
   dynamic configuration:
   The configuration file syntax changed and the utility command arguments changed.
   **Please note, the documentation is not yet updated to reflect all changes.**

- Switched to dynamic configuration concept: a simulation setup description is no
  longer necessary for running a simulation.
  All participants announce and register their services upon connecting to the registry.

    - Removed support of old config format from public includes ``ib/cfg/Config.hpp``

    - Removed CreateComAdapter (use CreateSimulationParticipant instead)

    + old:

    .. code-block:: c++

       auto comAdapter = ib::CreateComAdapter(config, "ParticipantName", domainId);

    + new:

    .. code-block:: c++

       auto participant = ib::CreateSimulationParticipant(config, "ParticipantName", domainId, true);

        
    - CreateSimulationParticipant takes the new configuration as first argument.
      To configure a synchronized participant, set the fourth argument to `true`.
    - Demos: 
        - Removed demos CanDemoNoSync, DynamicConfig, TickTickDone, GenericMessage, SimulationControl, SimulationControlNonBlocking as they are outdated
        - Updated remaining demos to use CreateSimulationParticipant and updated the configuration files.
    - Startup: 
        - IbSystemController (reference implementation) does not take JSON file as first argument anymore and takes all expected synchronized participants as command line arguments.

            .. code-block:: sh

             .\IbSystemController.exe 42 CanWriter CanReader
            
    - SystemController API:
      The system controller now expects a complete list of synchronized participants for proper working:
    
     :cpp:func:`ib::mw::sync::ISystemController::SetRequiredParticipants()`
        


.. admonition:: Note: the VIBE Network Simulator configuration format has changed!

  The format of the Network Simulator configuration has changed. Refer to the
  sample configuration files distributed with the VIBE-NetworkSimulator package.



- The network simulator's configuration was changed.
  Please note, that the documentation does not yet reflect these changes.
  The new configuration format resembles the ``NetworkSimulator`` block of the legacy config format but is stripped down to the minimum:
  it contains blocks for ``Switches`` and ``SimulatedNetworks`` (which were previously called ``SimulatedLinks``):

  + Sample of the new Network Simulator format:
    
    .. code-block:: javascript

        {
            "SchemaVersion": "1",
            "Description": "Small sample config with Link names from the VIB Demos",
            "Switches": [
                {
                    "Name": "FrontSwitch",
                    "Ports": [
                        {
                            "Name": "Port0",
                            "VlanIds": [1],
                            "Network": "FS-Port0"
                        },
                        {
                            "Name": "Port1",
                            "VlanIds": [1],
                            "Network": "FS-Port1"
                        }
                    ]
                }
            ],
            "SimulatedNetworks": [
                {
                    "Name": "CAN1",
                    "Type": "CAN"
                },
                {
                    "Name": "LIN1",
                    "Type": "LIN"
                },
                {
                    "Name": "FlexRay1",
                    "Type": "FlexRay"
                },
                {
                    "Name": "ETH1",
                    "Type": "Ethernet"
                },
                {
                    "Name": "FS-Port0",
                    "Type": "Ethernet"
                },
                {
                    "Name": "FS-Port1",
                    "Type": "Ethernet"
                }
            ]
        }

  The Network Simulator is shipped with two sample configuration files and
  the process invocation is the same: ``VIBE-NetworkSimulator ParticipantName path/to/configFile``

Compatibility with 3.6.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: The versions from 3.6.3 up to 3.6.16 changed a lot of key APIs and network compatibilties.

- Application binary interface (ABI): No 
- Application software interface (API): No
- Middleware network protocol (VAsio): No


[3.6.3] - 2022-01
--------------------------------

Removed
~~~~~~~
- removed the deprecated FastRtps middleware:
    - CMake: removed option `IB_MW_ENABLE_FASTRTPS`.
    - Demos: removed the FastRtps specific config files.
    - removed FastRtps specific integration tests.
    - API: removed deprecated `ib::CreateFastRtpsComAdapter` and `ib::CreateVAsioComAdapter`.
      Users should use the generic :cpp:func:`CreateComAdapter<ib::CreateComAdapter()>`,
      refer to :ref:`changelog-outdated-reference`.
    - ConfigBuilder: removed FastRtps configuration mechanism: `ib::cfg::ConfigBuilder::ConfigureFastRtps`.

  

[3.6.2] - 2021-12-16
--------------------------------

Fixed
~~~~~
- C-API: fix transmission of flags in CAN SendFrame (VIB-548).
- Links are now checked for unique endpoints (VIB-542).

Changes
~~~~~~~
- Remove FastRTPS submodule in main repository.
- cmake: make Linux detection more portable.
  We now use ``lsb_release`` to determine Linux Standard Base name and version of the distribution.
- C-API: refactoring as described in VIB-529.
  The experimental C API has been refactored to make use of a unified naming scheme.
  The following changes have been introduced:

  - All identifiers related to a specific object (or topic) now use the prefix ``ib_Topic_``.
    For instance, ``ib_CanController_Stop`` has been renamed to ``ib_Can_Controller_Stop``.
    The names are now unified for all functions and types, and for the 'topics'
    SimulationParticipant, Can, Ethernet, FlexRay, Lin.

  - The functions ending with ``_create`` or ``_destroy`` have been renamed to ``_Create`` or ``_Destroy``.


  
Added
~~~~~
- Added SerDes support library for easy serialization /deserialization of user data.
  Refer to `Demos/GenericMessage/GenericMessageDemoSerDes.cpp` for an example (VIB-514).

Compatibility with 3.6.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes


[3.6.1-QA] - 2021-12-10
--------------------------------
This is a Quality Assured Release.

Added
~~~~~
- The launcher now supports YAML formatted configuration files based on their file
  name extension ``.yaml`` or ``.yml``.
  This requires an installation of PyYAML (e.g., via ``pip install PyYAML``).
  Please note, that JSON schema validation with YAML files is not supported (VIB-526).
- Added the option ``--exclude-registry`` to the launcher.
  This will prevent the launcher from starting an IbRegistry process on its own (VIB-527).

Changed
~~~~~~~
.. admonition:: Note: the VIB-CANoe Add-on is now discontinued.

  The CANoe Add-on is affected by a known regression.
  Upcoming releases of Vector CANoe will support the IntegrationBus directly.

- The VIB CANoe addon is now discontinued.
  It is affected by **a known regression** and will not be part of this release.
  Users of the VIB-CANoe add-on are encouraged to remain on the previous quality
  assured release.
  The upcoming release of Vector CANoe 16 will support the IntegrationBus directly.

Fixed
~~~~~
- Fixed a regression in the ``Replay`` feature and the VIBE-NetworkSimulator support (VIB-544).

Compatibility with 3.6.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes 
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes

[3.6.0] - 2021-12-01
--------------------------------
Please note, the version jump from v3.4.x to v3.6.x was necessary due to internal
releases already having the v3.5.x tag.

Added
~~~~~
- Add experimental C-API for life-cycle and time synchronization (VIB-492).
- Add experimental C-API for FlexRay (VIB-505).
- Add experimental C-API for LIN (VIB-516).
- Internal VIB threads now set their name for easier debugging on WIN32 and posix
  platforms (VIB-524).

Changed
~~~~~~~

.. admonition:: Note: Public API changed
  
  Some rarely used public headers and `IComAdapter` methods were removed.
  User code should re-compile cleanly with these changes.
  
- Some header files were removed from the public ``include/ib`` directories.
  These `IIbTo*` headers are only for internal use.
  This change should not affect any users of the public API directly (VIB-511).

Complete list of removed header files from ``include/ib``:

  .. code-block:: sh

     include/ib/mw/IIbEndpoint.hpp
     include/ib/mw/IIbMessageReceiver.hpp
     include/ib/mw/IIbReceiver.hpp
     include/ib/mw/IIbSender.hpp
     include/ib/mw/sync/IIbToParticipantController.hpp
     include/ib/mw/sync/IIbToSyncMaster.hpp
     include/ib/mw/sync/IIbToSystemController.hpp
     include/ib/mw/sync/IIbToSystemMonitor.hpp
     include/ib/sim/can/IIbToCanController.hpp
     include/ib/sim/can/IIbToCanControllerProxy.hpp
     include/ib/sim/can/IIbToCanSimulator.hpp
     include/ib/sim/eth/IIbToEthController.hpp
     include/ib/sim/eth/IIbToEthControllerProxy.hpp
     include/ib/sim/eth/IIbToEthSimulator.hpp
     include/ib/sim/fr/IIbToFrBusSimulator.hpp
     include/ib/sim/fr/IIbToFrController.hpp
     include/ib/sim/fr/IIbToFrControllerProxy.hpp
     include/ib/sim/generic/IIbToGenericPublisher.hpp
     include/ib/sim/generic/IIbToGenericSubscriber.hpp
     include/ib/sim/io/IIbToInPort.hpp
     include/ib/sim/io/IIbToOutPort.hpp
     include/ib/sim/lin/IIbToLinController.hpp
     include/ib/sim/lin/IIbToLinControllerProxy.hpp
     include/ib/sim/lin/IIbToLinSimulator.hpp

- Directly sending messages on the :cpp:class:`IComAdapter<ib::mw::IComAdapter>` via `SendIbMessage(...)`
  is not possible anymore.
  Sending messages is now only supported via the specific service controllers,
  see :ref:`VIB API<sec:api-services>` for an overview (VIB-511).

  For example, code that relies on `IComAdapter::SendIbMessage(EndpointAddress, const T&)` should
  use an appropriate controller for the given type T:

  + old:
    
    .. code-block:: c++

       // Example for message type T = CanMessage
       CanMessage msg{};
       comAdapter->SendIbMessage(EndpointAddress{/*implementation detail*/}, msg);

 + new:
   
   .. code-block:: c++

      // Example for message type T = CanMessage
      auto* controller = comAdapter->CreateCanController();
      CanMessage msg{};
      controller->SendMessage(msg);
 

Complete list of methods removed from the interface ``class IComAdapter`` in ``include/ib/mw/IComAdapter.hpp``:

  .. code-block:: c++

     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::can::CanMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanControllerStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanConfigureBaudrate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanSetControllerMode& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::eth::EthMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthTransmitAcknowledge& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthSetMode& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::CycleStart& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferConfigUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::PocStatus& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::SendFrameRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::SendFrameHeaderRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::Transmission& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::WakeupPulse& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::LinControllerConfig& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::ControllerStatusUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::FrameResponseUpdate& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sync::NextSimTask& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::Tick& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::TickDone& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const logging::LogMsg& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, logging::LogMsg&& msg) = 0;
    
     virtual void OnAllMessagesDelivered(std::function<void(void)> callback) = 0;
     virtual void FlushSendBuffers() = 0;

.. admonition:: Note: FastRTPS build is disabled

  The FastRTPs middleware build is now disabled for the official Vector packages.

- The binary packages are built with the CMake flag ``IB_MW_ENABLE_FASTRTPS=OFF``.
  Calling a ``CreateComAdapter`` with an active middleware of ``FastRTPS`` will result
  in a runtime exception.
- MSVC: our CMakeSettings.json now directly supports building with Ninja.
- The transmit acknowledges in Ethernet were changed to work with a single participant.
  That is, in a trivial simulation sending an Ethernet message will be immediately
  acknowledged (VIB-490).

Fixed
~~~~~
- Config: prevent multiple statements of the same keyword, e.g., multiple ``"Links"``
  blocks now raise an error (VIB-528).
- Fix building with ``IB_BUILD_TESTS=OFF`` (VIB-536).

Compatibility with 3.4.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The API changes consist of internal headers which were removed from ``include/ib`` and
the removal of ``ComAdapter::SendIbMessage`` methods.

- Application binary interface (ABI): No 
- Application software interface (API): No
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes

[3.4.6] - 2021-11-16
--------------------------------

Changed
~~~~~~~

.. admonition:: Note:

   The FastRTPS middleware is now deprecated
  
- The middleware `FastRTPS` is now marked as deprecated.
  This middleware will be removed in the future.
  The middleware specific `CreateFastRtpsComAdapter` API has been
  marked as deprecated for a long time.
  Users should adopt the generic :cpp:func:`CreateComAdapter<ib::CreateComAdapter()>`.
- C-API for ethernet was refactored and improved (VIB-489).
- C-API for CAN was refactored and internal integrity checks and unit tests
  added (VIB-464 VIB-465 VIB-466 VIB-460 VIB-462).
- The transmit acknowledges in CAN were changed to work with a single participant.
  That is, in a trivial simulation sending a CAN message will be immediately acknowledged (VIB-473 VIB-478).
- Updated gtest to version 1.11. This version supports a simplified
  MOCK_METHOD macro with specifiers like `const, override`. (VIB-474).

Added
~~~~~
- Added new functional tests for CAN and Ethernet without simulation control
  flow / synchronization (VIB-491 VIB-494).
- A new C-API for a DataPublisher / Subscriber (VIB-432).

Fixed
~~~~~
- Fixed regression in ``Pause()/Continue()`` calls of ParticipantController.
  The `SimulationControl` Demo was added and used as a stress test (AFTMAGT-330 VIB-499).

Compatibility with 3.4.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (interface change due to transmit acknowledges)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.5] - 2021-11-03
--------------------------------

Added
~~~~~
- C-API: added initial ethernet bindings for the C language.
  Refer to :ref:`sec:capi` for documentation.
  Currently, the Ethernet controllers work without time-synchronization.
  A Demo is provided in ``Demos/CEthernet`` (VIB-431).

Fixed
~~~~~
- Clarify the documentation of synchronization types and the usage of
  ``SetPeriod()`` (AFTMAGT-329).
- Fix YAML conversions with the ``vib-config-tool``.
  The YAML parser was emitting JSON formatted input back in JSON format.
  Ensure that default config values are properly serialized, or if unchanged are
  omitted from the output (AFTMAGT-331).
- Fix VAsio config values in the JSON schema.
  Update the ConfigBuilder to support the latest config additions.
  Clarified the documentation, which indicated misplaced config items nested in the
  ``Registry`` config item (AFTMAGT-328).
- Ensure user input (in different encodings) is not used verbatim in the
  domain socket creation (VIB-459).


Compatibility with 3.4.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.4] - 2021-10-20
--------------------------------

Added
~~~~~
- Added initial C language bindings. Please note, the C API is currently
  work in progress and as such might change in the future.
  The documentation is in :ref:`sec:capi`.
  C language bindings exist for CAN controllers without time-synchronzition
  and creating simulation participants from config files.

Compatibility with 3.4.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.4.3-QA] - 2021-09-15
--------------------------------
This is a Quality Assured Release.

Added
~~~~~
- CMake: the options IB_MW_ENABLE_VASIO and IB_MW_ENABLE_FASTRTPS can be used to
  disable the middlewares at compile time.
- Config: introduced a 'SchemaVersion' field in the config formats, to allow for future
  changes.

Fixed
~~~~~
- VAsio: fix a bug affecting distributed (networked) setups. Participants did
  only listen on localhost addresses and the registry did not distribute proper
  endpoint addresses to other participants.

.. admonition:: Note: this bug affects users of VAsio with networked setups

  Users with VIB setups across multiple hosts (with non-localhost addresses)
  should upgrade to this VIB version.

- VAsio: improve performance. The `TraceTx()`/`TraceRx()` invocations were expanding
  their arguments and applying a `to_string()` operation on the VIB message
  payloads. Now, this is code path is only executed when `Trace` log level is active.

- UB sanitizer: fixed undefined behavior in MessageBuffer (unaliased memory
  accesses) and Watchdog (integer overflow).

- Fixed missing 'ActiveMiddleware' statements in Can/FastRTPS config files and
  ensure it is properly parsed with the new YamlConfigParser (VIB-375).

- Fixed out of bounds exception when configuring tracing and replaying.
  Also ensure that configuring a replay block with a 'Receive' direction is able
  to replay messages (VIB-372).

- Ensure that 'SearchPathHints' are passed on to the VIBE extension implementing
  the IReplayDataProvider interface (VIB-378).

- Fix accidentally setting the badbit on the ostream when formatting Ethernet
  MAC addresses and LIN frames.

Changed
~~~~~~~
- ``Config::FromJsonString`` now uses a Yaml-parser internally. To update legacy
  JSON configs to the current valid format we provide the ``vib-config-tool``
  utility. It supports deprecated controller and port declarations (cf `[3.1.0] - 2020-06-15`_)
  and also network simulator configurations at `SimulationSetup` scope (cf `[3.3.0] - 2020-08-12`_).
  It can output in `JSON` or `YAML` format. For example, to ensure a JSON config
  is up to date:

  .. code-block:: sh

     vib-config-tool --convert oldFile.json updated.json --format json

.. admonition:: Note: the IbRegistry executable was moved again from the `IntegrationBus-NonRedistributable` directory to the `IntegrationBus/bin`

    The relocation of the registry to the NonRedistributable caused so much
    irritation and breakage that we undid this change.

- To reduce the fallout of moving the IbRegistry we put it back into
  `IntegrationBus/bin/`. For compatibility the IbLauncher searches the registry
  executable in both directories.

Removed
~~~~~~~

- Removed the json11 from the source-tree. We now use yaml-cpp to parse JSON and YAML
  configuration files. Please note, that the yaml-based parser still accepts
  JSON formatted inputs.

Compatibility with 3.4.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (json11 symbols removed)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.2] - 2021-08-04
--------------------------------

Added
~~~~~

- VAsio now supports UNIX domain sockets as transport. By default, all participants
  open domain sockets locally and advertise the TCP/IP and local endpoints to other participants.
  If a participant is unable to connect to another participant locally, the TCP/IP
  endpoint is used for connecting without raising an error.
  As such, the local domain connectivity is optional and fully transparent to the users.
  Because of the path limitations of domain sockets the socket files are stored
  in the system temporary directory (e.g., %TEMPDIR% or $TMP).
  To allow for simulation isolation, the socket files have the participant IDs,
  domain ID, and the simulation's working directory encoded in the file name.

- The CMake build system is now better equipped for reproducible builds on Linux platforms.
  CI builds now set the TZ, LANG and SOURCE_DATE_EPOCH environment variables to known values.
  The time of the last commit on the master branch is used to initialize SOURCE_DATE_EPOCH.
  Packaged source-trees now contain a pre-processed version_macros.hpp recording the commit
  hash.

Fixed
~~~~~

- Various fixes for our scripted Jenkins pipeline to improve stability on our CI environment.

Changed
~~~~~~~

- VAsio: we imported the Asio library to version 1.18.2.
  This includes many bugfixes and also domain socket support for Windows 10.
  The FastRTPS packages used for CI/CD builds had to be rebuilt using this new version of
  Asio.

- CMake: the third party handling code was refactored. Instead of using
  variables we now have targets to link against the third party components.

Compatibility with 3.4.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (additions to `struct ib::cfg::VAsio::Config`)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.1] - 2021-06-25
--------------------------------

Added
~~~~~
- Added native YAML parser implementation.
  This provides better error messages and warnings for parsing YAML and JSON config files
  via the ``Config::FromYamlString``/``Config::FromYamlFile`` procedures.
  When the parser encounters an unknown element it emits a warning.
  However, if the element uses a reserved element name of the config 
  schema the warning is turned into an error (AFTMAGT-314).
  Please note, that schema versioning for backward compatibility is
  not yet implemented and will be addressed in the future.

Fixed
~~~~~
- Fixed warnings during 32-bit builds: an int64_t was truncated to int.
- Build: relax some warning levels for gtest and remove useless compile
  flags on windows.

Changed
~~~~~~~
- When shutting down, do not print an error message that we're already
  shutting down.

.. admonition:: Note: the IbRegistry executable was moved to the NonRedistributable directory.

  The location of the IbRegistry executable now matches that of the vib-registry shared library,
  cf. :ref:`sec:util-registry`.

- Similar to the vib-registry shared library, the executable was moved to the
  IntegrationBus-NonRedistributable directory.



Compatibility with 3.4.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.4.0] - 2021-06-01
--------------------------------

Added
~~~~~
- Added submodule yaml-cpp. This will be the base of our native
  YAML configuration parser.
- Added ``ib::cfg::Config::FromYamlString`` and ``ib::cfg::Config::FromYamlFile`` to load
  configuration from YAML formatted input. This currently transforms
  the YAML input into JSON and re-uses the JSON config parser.

Compatibility with 3.3.10
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): No (additions to ``ib::cfg::Config``)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.10-QA] - 2021-05-10
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- Added MdfChannel identification to replaying config.
  This allows replaying MDF4 trace files that do not originate from VIB
  simulation runs, cf. :ref:`changelog-outdated-reference` (``sec:replay-foreign``).

Fixed
~~~~~
- Allow tracing while replaying on I/O InPorts (VIB-159).
- Allow tracing while replaying on GenericSubscribers (VIB-159).
- Fix trivial FlexRay simulation. State transitions when a wakeup command was issued were not properly computed (VIB-154).
- Ensure that only active replay controllers are configured.
  This fixes a crash when multiple controllers were defined, but only one was active (VIB-160).
- Allow tracing messages on a LIN master when replaying is active (VIB-158).
- Fix null pointer derference in PcapReader when input file name was missing in configuration.
  Also ensure that the config has non-empty input and output file paths (VIB-156).
- The Launcher will attempt to clean up the CANoe environment several times when shutting down.
  This ensures a clean CANoe installation when CANoe is slow to shutdown, e.g., when launcher is interrupted
  by a user (VIB-153).
- Fix LIN Sleep frames when using the VIBE Network Simulator.
  When the ``ILinController::GoToSleep()`` was invoked, a `sleep` frame was transmitted and
  then the controller's internal state was set to `Sleeping`. This caused the Network Simulator
  to abort the running sleep-frame transmission and an erroneous  LIN_RX_NO_RESPONSE frame was generated.
  An additional `sleep pending` state is introduced which allows completion of pending transmissions before entering
  the `sleep` state. (VIB-155)

  .. admonition:: VIBE Network Simulator compatibility 

     To ensure interoperability you should use VIBE Network Simulator v3.3.10
     in all setups involving detailed LIN simulations.
     See compatibility below for details.

  
Changed
~~~~~~~
- Print acknowledgement on std::cout when an extension is loaded.
- Throw IB exceptions instead of runtime_error where applicable.
- Update the Demo configs to newer ``NetworkSimulator`` configuration scheme (VIB-156).
- Updated JSON config schema to include ``TraceSource`` and ``Replay`` config blocks (VIB-156).


Compatibility with 3.3.9
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

Please note that the detailed simulation of LIN requires a matching VIBE Network Simulator v3.3.10.
The addition of a new internal state makes the current VIB release incompatible with older Network Simulators for the detailed LIN simulation.


- Application binary interface (ABI): No (Due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.9] - 2021-04-09
--------------------------------

Added
~~~~~
- Add documentation for :ref:`changelog-outdated-reference` (``Replaying and Tracing<usage/replay>``) (AFTMAGT-308).
- Added support for replaying FlexRay messages (AFTMAGT-289).
  Replaying FlexRay requires the use of the VIBE-NetworkSimulator.
  The NetworkSimulator is attached as a replay controller to the replay scheduler, cf. :ref:`changelog-outdated-reference` (``sec:replay-architecture``) for an overview.
- Added support for replaying FlexRay messages in the VIBE-NetworkSimulator (AFTMAGT-290).
  Please note, that the startup/synchronization sequence is not part of a trace file  and time stamps of the
  replay might deviate from the original traced messages.
  The logical order of messages is kept after the synchronization has been established.
  Refer to section :ref:`changelog-outdated-reference` (``sec:replay``) for a summary of supported features and limitations.
- Add a :ref:`changelog-outdated-reference` (``Replay<sec:cfg-participant-replay>``) configuration block to the NetworkSimulator
  configuration.

Fixed
~~~~~
- The config parser no longer uses asserts when validating a Config.
  A misconfiguration exception is now thrown, which can be handled by the user (AFTMAGT-309).

Compatibility with 3.3.8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

- Application binary interface (ABI): No (Due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.8] - 2021-02-18
--------------------------------

Added
~~~~~
- The CI build pipeline now includes Ubuntu 20.04 as a build target, which is used
  to run automated unit tests and integration tests (AFTMAGT-300).
- Added support for replaying LIN frames using the VIBE-MDF4Tracing extension.
  This is exclusively supported on LIN master controllers.
  Replaying on LIN slaves is not supported, even though tracing is possible
  on these controllers.
  Invoking transmission API calls during replay is also not supported on a master (AFTMAGT-288).
- Setting VAsio TCP flags is now supported from a JSON config file, cf. :ref:`changelog-outdated-reference` (``sec:mwcfg-vasio``) (AFTMAGT-305).


Fixed
~~~~~
- A graceful connection shutdown does no longer result in an error message when
  using the VAsio  middleware (AFTMAGT-299).
- Starting multiple VAsio registries on the same TCP/IP port resulted in
  empty error messages. Now, useful error messages are reported.
  On windows no error was reported at all. The registry listening socket is now
  created with SO_EXCLUSIVEADDR on windows, which prevents mulitple registries
  to share the same listening port (AFTMAGT-303).


Compatibility with 3.3.7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.7] - 2021-01-27
--------------------------------

Added
~~~~~
- Added replaying functionality for Ethernet, Can, GenericMessages and IO ports.
  This utilizes a new ReplayScheduler and replay controllers to inject replay
  messages from a trace file. The PCAP file format  is supported natively, and
  the MDF4 format is supported through the VIBE-MDF4Tracing extension.

Changed
~~~~~~~
- The semantics of the :cpp:func:`ComAdapter::Create*<ib::mw::IComAdapter::CreateCanController>`
  methods to create services has changed.
  Previously, invoking them multiple times would result in an exception.
  Now, the same pointer is always returned.
  The ReplayScheduler uses these methods to create and configure replay
  controllers for services internally.

Fixed
~~~~~
- When setting the ``struct ib::cfg::VAsio::Config::tcpQuickAck`` option, the
  TCP_QUICKACK socket option is now activated after each successful read()/recvmsg on
  Linux. This ensures that the default 40ms delayed acknowledge timeout is not
  used.

Compatibility with 3.3.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.6-QA] - 2020-12-10
--------------------------------

This is a Quality Assured Release.

Fixed
~~~~~
- Utility executables are now built with appropriate RPATHs on Linux.
- Remove -Wabi flags from gcc/clang builds, as they were not correctly used
  and they break the build on Ubuntu 20.04.
- Update the config schema to include the IbRegistry logger.

Compatibility with 3.3.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.5] - 2020-11-26
--------------------------------

Added
~~~~~~
- The number of connection attempts can now be configured in the VAsio registry
  configuration, see the "ConnectAttempts" field in :ref:`changelog-outdated-reference` (``sec:mwcfg-vasio``).
- Added preliminary TCP/IP tuning settings to ``struct ib::cfg::VAsio::Config``. 
  The following settings are available:

  * tcpNoDelay: enable the TCP_NODELAY flag, which disables Nagle's algorithm (default off).
  * tcpQuickAck: enable the Linux specific TCP_QUICKACK, which disables delayed acknowledgements (default off).
  * tcpSendBufferSize and tcpReceiveBufferSize: set the TCP buffer sizes.

Changed
~~~~~~~
- The TCP_NODELAY is now off by default again. It can be enabled using the VAsio
  config.

Fixed
~~~~~
- The IbLauncher now considers debug libraries when searching for the VIB
  installation, this makes it usable with Debug builds.

Compatibility with 3.3.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (due to Config changes)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.4] - 2020-11-04
--------------------------------

Added
~~~~~~
- The CMake build system has new options to build with sanitizers:
  IB_ENABLE_ASAN and IB_ENABLE_UBSAN, to enable 'Address Sanitizer' and
  'Undefined Behavior Sanitizer', respectively.
  When active, the -fsanitize=... compile options and link options are enabled
  globally.
  This is currently only supported on GCC and Clang.
- Added replay message provider interfaces for the upcoming MDF replaying
  extension.

Fixed
~~~~~~
- Fixed undeclared variable use in IbLauncher (AFTMAGT 294).

Changed
~~~~~~~
- Added the TCP_NODELAY option to VAsio (AFTMAGT 297).
  This reduces latencies when sending a lot of small VIB messages.

Compatibility with 3.3.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.3] - 2020-10-15
--------------------------------
Added
~~~~~~
- Added a new configuration format for replaying traces,
  refer to :ref:`changelog-outdated-reference` (``sec:cfg-participant-replaying``) for details.
  Please note that the replaying mechanism is still under development.

Changed
~~~~~~~
- The IbRegistry command line utility now supports a ``--use-signal-handler`` flag
  that prevents it from listening on stdin. It can be safely shut down with
  Control-C when started with this flag.

Compatibility with 3.3.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

- Application binary interface (ABI): No (due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.2] - 2020-09-24
--------------------------------

Changed
~~~~~~~
- The duplicate IbConfig schema was removed from the Launcher subdirectory.
  There is now only one instance of IbConfig.schema.json under
  IntegrationBus/source/cfg/.
- Integration tests were refactored and stabilized (AFTMAGT 271).
  Networked integration tests now use a synchronized simulation, with a new test
  harness that simplifies test case setup.

Compatibility with 3.3.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.1-QA] - 2020-08-27
--------------------------------

This is a Quality Assured Release.

Fixed
~~~~~
- Fixed config JSON schema and updated Demos to latest configuration syntax.
- Allow 'UseTraceSinks' when parsing configuration files in backward-compatibility
  mode.
- Make trace sink attachment deterministic per participant. This
  changes an internal interface used to load the VIBE mdf4tracing extension.
- Fix launcher when there is a trailing separator in the environment PATH
  variable.

Changed
~~~~~~~
- Building the documentation now requires Sphinx version >= 3.0 (cf. :doc:`../development/build`).
  
Compatibility with 3.3.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.0] - 2020-08-12
--------------------------------

Added
~~~~~
- Added support for tracing bus messages on the VIBE-NetworkSimulator (AFTMAGT 277).
- Enable tracing for GenericMessages (AFTMAGT 233).
- Enable tracing for IoPorts (AFTMAGT 276).
- Added support for tracing CAN/LIN/Ethernet bus messages on controller proxies (AFTMAGT 278).

Changed
~~~~~~~
- The NetworkSimulator configuration syntax was changed. The network simulator
  definition was moved from the SimulationSetup level, down to the participant
  that previously only referred to the network simulator by name.
  For backward compatibility the old configuration syntax is still supported.
  (AFTMAGT-277).

  + old:
    
    .. code-block:: javascript

       "SimulationSetup": {
           "Participants": [
                {
                    "Name": "NetworkSimulator",
                    "NetworkSimulators": [ "NetSim1" ]
                }
           ],
           "NetworkSimulators": [
                {
                    "Name" : "NetSim1",
                    "SimulatedLinks": [...]
                }
           ]
        } 

  + new:
    
    .. code-block:: javascript


       "SimulationSetup": {
           "Participants": [
                {
                    "Name": "NetworkSimulator",
                    "NetworkSimulators": [
                        {
                            "Name" : "NetSim1",
                            "SimulatedLinks": [...]
                        }
                    ]
                }
           ]
        } 
  + This change also affects the config builder API and ib::cfg::Config.



Compatibility with 3.2.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The config builder API for network simulators was changed, and the struct
ib::cfg::Config was also modified.

- Application binary interface (ABI): No  (changes in Config)
- Application software interface (API): No (changes in Config)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.2.1] - 2020-07-23
--------------------------------

Added
~~~~~

- The startup delay of a FlexRay node in the FlexRay demo can now be set programmatically.
- Added a message tracing section to :ref:`changelog-outdated-reference` (``Participant Configuration<sec:cfg-participant-tracing>``).
  It briefly discusses how to configure the trace sink mechanism and how the 
  :ref:`changelog-outdated-reference` (``mdf4tracing``) extension is used.
  The controllers now also reflect the recently updated tracing support.

Changed
~~~~~~~

- Enabled message tracing on the following controllers: LIN, CAN, FlexRay.
- The Ethernet demo now uses the IEthController::SendFrame API.
- Updated version of the third-party library `fmt` to version `6.1.0`.

Fixed
~~~~~

- Fixed a bug in the FlexRay demo which caused unreachable code in the POC Handler.
- TraceSinkBuilder was missing API exports for three methods, resulting in
  missing symbols when linking on Windows.
- Fixed shared library loading incompatibilities.

Compatibility with 3.2.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No 
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.2.0] - 2020-07-06
--------------------------------

Added
~~~~~

- Logger of the VAsio Registry can now be configured via the middleware configuration,
  cf. :ref:`changelog-outdated-reference` (``sec:mwcfg-vasio``). The corresponding :cpp:class:`RegistryBuilder<ib::cfg::VAsio::RegistryBuilder>`
  also gained the :cpp:func:`ConfigureLogger()<ib::cfg::VAsio::RegistryBuilder::ConfigureLogger>` method to configure
  the logger of the VAsio Registry.
- Added benchmark demo, cf. :ref:`changelog-outdated-reference` (``sec:util-benchmark-demo``).

.. _sec:vib320-changed:

Changed
~~~~~~~

- :cpp:class:`ib::type_conversion_error<ib::type_conversion_error>` inherits now from 
  :cpp:class:`std::runtime_error<std::runtime_error>` instead of :cpp:class:`std::exception` directly.
- :cpp:class:`ib::cfg::LoggerBuilder<ib::cfg::LoggerBuilder>` doesn't inherit from
  :cpp:class:`ib::cfg::ParentBuilder<ib::cfg::ParentBuilder>` anymore.

Fixed
~~~~~
- Fixed a bug that prevented legacy IbConfigs (pre VIB 3.1.0) using the PcapFile
  setting from being converted to new IbConfigs (VIB 3.1.0) using trace sinks.


Compatibility with 3.1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No (See :ref:`Changed<sec:vib320-changed>`)
- Application software interface (API): No (:ref:`LoggerBuilder API changed<sec:vib320-changed>`)

- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.1.0] - 2020-06-15
--------------------------------

Added
~~~~~
- New optional configuration section for extension-related settings,
  cf. :doc:`../configuration/extension-configuration`. Its only property
  is the list of extension search path hints, which allows to configure
  the additional search paths for shared library extensions loaded by the VIB.

- New configuration mechanism for IB message tracing.
  It supersedes the previous Ethernet and PCAP specicic configuration, please
  refer to the Deprecated section.

- The ParticipantBuilder gained a new AddTraceSink() method, which returns
  a TraceSinkBuilder. A TraceSink consists of a unique, non-empty name, an output 
  path and the format type (PcapFile, PcapPipe, Mdf4File).

- Services, Controllers and Ports can be configured to use a trace sink by name.
  For example, by invoking the
  :cpp:func:`WithTraceSink(name)<ib::cfg::GenericPortBuilder::WithTraceSink()>`
  on the appropriate builder.
  This will populate the 'UseTraceSinks' field of the JSON serialization of the
  builder's configuration type.

Changed
~~~~~~~
- For FastRTPS, the default participant lease duration is now 2h to avoid
  connection losses when debugging. (AFTMAGT-267)

- To enable the newly added 'UseTraceSinks' fields, the JSON serialization
  format of the Controllers, Services and Ports were adjusted.
  In particular, the JSON type of DigitalIoPort, AnalogIoPort, PwmPort,
  PatternPort, and GenericSubscriber were changed:

  + old:

    .. code-block:: javascript

       "Port-Type": [ "PortName", "OtherPort", ...]

  + new:

    .. code-block:: javascript

       "Port-Type": [
            {
                "Name":  "PortName"
            },
            {
                "Name":  "OtherPort"
            }
       ]




Fixed
~~~~~
- :cpp:func:`ILinController::SendFrameHeader(LinIdT)<void
  ib::sim::lin::ILinController::SendFrameHeader(LinIdT)>` now correctly sets the
  current simulation time in the LinTransmission. Previously, the timestamp was
  always 0s.

- GenericSubscriber was missing the ITimeProvider interface.

.. _sec:api-withpcap-removed:

Removed
~~~~~~~
- The ControllerBuilder<EthernetController> no longer supports the
  WithPcapFile() and WithPcapPipe() methods.
  This usage has been superseded by the new configuration mechanism:

  + old:

    .. code-block:: c++

        simulationSetup
            .AddParticipant("P1")
            .AddEthernet("ETH1")
            .WithPcapFile("output filename");

  + new:

    .. code-block:: c++

        auto&& participant = simulationSetup.AddParticipant("P1");
        participant->AddEthernet("ETH1").WithTraceSink("EthSink");
        participant->AddTraceSink("EthSink")
            .WithType(TraceSink::Type::PcapFile)
            .WithOutputPath("output filename.pcap");



Deprecated
~~~~~~~~~~
- The 'pcapFile' and 'pcapPipe' fields in the EthernetController configuration 
  are deprecated. Please use the newly added 'UseTraceSinks' and 'TraceSinks' 
  fields. These fields will be removed from the JSON format and the Config
  Builder API in the future.

  + old:
    
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "PcapFile": [ "EthernetReader.pcap" ]
           }
        ] 

  + new:
    
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "UseTraceSinks": "EthernetSink"
           }
        ] 
        "TraceSinks": [
            {
                "Name" : "EthernetSink",
                "OutputPath": "EthernetReader.pcap",
                "Type": "PcapFile"
            }
        ]

- Loading a JSON file which contains the deprecated "PcapFile" or "PcapPipe"
  fields will cause a runtime warning. Internally the data structures  are updated
  as if a "TraceSinks" and "UseTraceSinks" was supplied with a TraceSink name 
  that is derived from the Participant's and EthernetController's names.

Compatibility with 3.0.7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No (:ref:`ConfigBuilder API changed<sec:api-withpcap-removed>`)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.7] - 2020-05-25
--------------------------------

Added
~~~~~
- Add a time provider interface internal to the service controllers.
  By default the wallclock time is used as the source for the current time.
  When a participant controller is present, its virtual simulation time is used.
- IEthController gained a new API for sending Ethernet frames with explicit and
  implicit timestamps:
  :cpp:func:`IEthController::SendFrame(EthFrame, nanoseconds)<EthTxId ib::sim::eth::IEthController::SendFrame(EthFrame, std::chrono::nanoseconds)>`
  and 
  :cpp:func:`IEthController::SendFrame(EthFrame)<EthTxId ib::sim::eth::IEthController::SendFrame(EthFrame)>`.

  These methods will support MDF4 tracing in the future.
  The controller's time provider will be queried if no user supplied timestamp
  is present.

- ParticipantControllers can now forcefully exit the run loop in case of an
  error:
  :cpp:func:`IParticipantController::ForceShutdown()<ib::mw::sync::IParticipantController::ForceShutdown()>`.
  Note that this method is only intended for use cases where a regular shut down
  is not possible!

Fixed
~~~~~
- Fixed a memory leak: there was a shared pointer cycle in SyncMaster's
  DiscreteTimeClient when attaching a lambda to itself.

Deprecated
~~~~~~~~~~
- Please note, that the :cpp:func:`IEthController::SendMessage(EthMessage)<EthTxId ib::sim::eth::IEthController::SendMessage(EthMessage)>`
  method is deprecated in favor of the new SendFrame() methods.
  It will be removed in the future.
  The EthMessage struct contains a user-settable timestamp, which is not a good
  fit for the new time provider based API.

Changed
~~~~~~~
- CMake build: the binaries are now all built in the
  ${CMAKE_BINARY_DIR}/$<CONFIG> directory. This allows running Demos and Tests
  directly from the build directory, which eases debugging.

Compatibility with 3.0.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.6] - 2020-04-30
--------------------------------

Fixed
~~~~~
- Disable remote logging when shutting down (AFTMAGT252)
  This fixes issues when remote logging is enabled with log levels of debug and
  higher. The FastRTPS middleware uses debug log messages internally, and during
  shutdown the LogMsg FastRTPS topic is unmatched and destroyed -- which leads
  to invalid accesses when remote logging is enabled.

- PCAP tracing now includes the ingress data on EthControllers (AFTMAGT265).

Changed
~~~~~~~

- We no longer bundle FastRTPS binaries in the official VIB packages.
  Users had issues using the exported cmake targets from FastRTPS binaries when
  building from source.
  The VIB cmake build system fetches FastRTPS using git when the FastRTPS
  depdendencies are missing from the local source tree.

Compatibility with 3.0.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.5] - 2020-04-08
--------------------------------

Added
~~~~~
- It is now possible to provide a time stamp for LIN transmissions. For this,
  overloads have been added to
  :cpp:func:`ILinController::SendFrame()<void ib::sim::lin::ILinController::SendFrame(Frame, FrameResponseType, std::chrono::nanoseconds)>`
  and
  :cpp:func:`ILinController::SendFrameHeader()<void ib::sim::lin::ILinController::SendFrameHeader(LinIdT, std::chrono::nanoseconds)>`.
  Note that this timestamp will be overwritten when using the VIBE NetworkSimulator.

- The VAsio registry can now be used as a shared library. Please note that the
  shared library is non-redistributable.
  The extension mechanism will load the shared library and construct an instance
  of the :cpp:class:`IIbRegistry` interface for the user to consume.
  The API entry point is the
  :cpp:func:`CreateIbRegistry()<ib::extensions::CreateIbRegistry>` function.
  The vib-registry shared library needs to reside in the current process's 
  working directory.
  Initially, it is located in the ``IntegrationBus-NonRedistributable``
  subdirectory of the VIB package.
  In case of error a std::runtime_error is thrown.

Changed
~~~~~~~
- Don't format Logger messages if the messages aren't going to be
  logged anyways.

Compatibility with 3.0.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.4] - 2020-03-19
--------------------------------

Added
~~~~~
- A new FlexRay controller API is introduced for monitoring protocol operation
  control (POC) status changes, cf. :ref:`sec:poc-status-changes`.
  This new API supersedes the  ControllerStatusHandler mechanism, as current use
  cases and new ones are covered by the PocStatus handler.
  This new API exposes more status variables of the POC when using the VIBE
  Network Simulator for FlexRay simulation. (AFTMAGT-253)

  .. admonition:: Note

     To ensure interoperability you should use VIBE Network Simulator v3.0.4
     in all setups involving different, but compatible versions of VIB.
     See compatibility below for details.

Changed
~~~~~~~
- Clarify error messages on connection loss for VAsio. The previous term
  "Shutdown" was ambiguous, the error reason now states "Connection
  lost" (AFTMAGT-260).
- Disable problematic FastRTPS connection loss detection (AFTMAGT-259).


Deprecated
~~~~~~~~~~
- :cpp:func:`IFrController::RegisterControllerStatusHandler()<ib::sim::fr::IFrController::RegisterControllerStatusHandler()>`
  is now deprecated in favor of
  :cpp:func:`RegisterPocStatusHandler()<ib::sim::fr::IFrController::RegisterPocStatusHandler()>`.
  ControllerStatusHandler will be removed in a future release.
  The usage of RegisterControllerStatusHandler will result in a warning
  at runtime and compile time.


Fixed
~~~~~
- The IbRegistry can now be used for multiple simulation runs without the need
  to terminate and restart it (AFTMAGT-249).
- The internally used spdlog is now build with compiler flags that prevent
  creation of weak symbols on Linux / GCC (AFTMAGT-256).


Compatibility with 3.0.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please note that the new FlexRay controller model (VIB v3.0.4) requires the
matching VIBE Network Simulator v3.0.4, even when not using the new PocStatus
API. However, the VIBE Network Simulator v3.0.4 is fully compatible with
previous FlexRay controller models and enables interoperability between VIB
v3.0.3 and v3.0.4 participants.

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.3] - 2020-02-26
--------------------------------

Added
~~~~~
- New integration test ensuring that VAsio is fully deterministic and delivers messages strictly in-order.
  This test replaces the demo GenericMessageITest, which is now obsolete and has been removed for this reason.

Fixed
~~~~~
- ib::version::Patch() was set to a wrong value.

Removed
~~~~~~~
- Removed demo GenericMessageITest.

Compatibility with 3.0.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.2-QA] - 2020-02-10
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- Documentation for the CAN controller API: :doc:`CAN Vehicle Network Controllers <api/can>`.
- Documentation for the Participant Controller API: :ref:`changelog-outdated-reference` (``api/participantcontroller``) (AFTMAGT-206).
- Documentation for the IO Port services (AFTMAGT-201).
- Documented Generic Messages API: (AFTMAGT-204).
- Documented the simulation state machine and synchronization types: :doc:`simulation/simulation`
- Added docs for the ComAdapter:
- Added quick start guide: usage/quickstart
- Elaborate the user APIs and overview pages: :doc:`api/api`
- Add docs for :doc:`api/systemcontroller` (AFTMAGT-242).
- Add docs for :doc:`api/systemmonitor` (AFTMAGT-242).
- Add docs for :doc:`api/ethernet` (AFTMAGT-239).

Changed
~~~~~~~
- Removed the upper limit of the VAsio send Queue, to avoid that critical
  IbMessages are thrown away. (AFTMAGT-240)
- VIB Utilities are now distributed in Release build configuration (AFTMAGT-245)
- The interactive system controller has been promoted from demo to
  utility. I.e., it is now part of the binary delivery.

Fixed
~~~~~
- The LIN controller now only calls the goToSleepHandler if the frame id and
  also the data field of a received LIN frame matches the id and data of a valid
  "GoToSleep" frame (AFTMAGT-244).
- VIB applications built in RelWithDebInfo or MinSizeRel will no longer link
  against the VIB Debug installation, which crashes under windows. (AFTMAGT-246)
- Fixed compilation errors caused by windows.h, which broke std::min/max calls
  in VIB headers. (AFTMAGT-248)
- Fixed a crash when move assigning the ConfigBuilder. (AFTMAGT-24)
- Fixed the cmake exported targets. CMake users should be able to use
  find_package(CONFIG) to integrate with the IntegrationBus library.
- Fixed a bug that prevented the CAN controller state callback from being called
  when using VIBE NetworkSimulator.
- Removed a false warning for VAsio with syncType::DistributedTimeQuantum, which
  incorrectly informed a user that IParticipantController::setPeriod() has no
  effect for this syncType.

Compatibility with 3.0.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.1] - 2020-01-08
--------------------------------

Added
~~~~~
- New config parameter "HistoryDepth" added to FastRTPS config section. This value is used to
  set the history size for all FastRTPS topics.
- New WithHistoryDepth method for FastRtpsConfigBuilder. When using the builder pattern to
  generate an Ib Config, the new FastRTPS HistoryDepth can be configured this way.
- New documentation for the configuration mechanism, cf. :doc:`../configuration/configuration`
- New documentation for FastRTPS configuration, cf. :doc:`../configuration/middleware-configuration`
- Extend the simulation setup documentation, cf. ../configuration/simulation-setup

Changed
~~~~~~~
- The IbLauncher CANoe environment module was adapted to the new CANoe Extensions packaging format.

Fixed
~~~~~
- A check was added to prevent participants from using ParticipantID 0, since this ID is reserved for the Registry when VAsio is used.

Compatibility with 3.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.0] - 2019-12-03
--------------------------------
Added
~~~~~
- The logging mechanism now includes the most important elements of sent and received IB messages (AFTMAGT-217).

Changed
~~~~~~~
- The VIB distribution uses a new directory layout for packages:

  IntegrationBus
    Contains the pre-built binary distribution of the IntegrationBus, including the C++ header files and CMake export targets.
    
  IntegrationBus-Demos
    The VIB demos are now distributed in source form.
    They can be easily compiled against the distributed VIB binaries: e.g, on Windows just right-click and "open in visual studio"
    
  IntegrationBus-Documentation
    Contains the documentation in HTML and text format.
    
  IntegrationBus-Source
     The VIB source tree, with notable changes:
     
     - The SystemController and SystemMonitor demos were moved to the Utilities/ directory. 
       They are also distributed in binary form.
     - Fast-RTPS is no longer distributed in source form. 
       The CMake build infrastructure automatically downloads a git snapshot if needed (requires git).

- CMake packaging was simplified (AFTMAGT-195).
- A notification is shown when writing to a PCAP pipe is enabled, as the default behavior is to block until the pipe is read by another process (AFTMAGT-221).
- Logging output to std::cerr and std::cout is replaced by calls to the internal logging mechanism (AFTMAGT-210).
- Warn user when a PCAP pipe is opened (AFTMAGT-221).
       
  
Fixed
~~~~~
- Fixed the Participant subscription in VASio (AFTMAGT-216). Creating an IB service will now block until all the necessary subscriptions have been acknowledged by all known participants.

Compatibility with 2.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): No

[2.0.0] - 2019-11-06
--------------------------------
Added
~~~~~
- New watchdog functionality for SimTasks with soft and hard limits. Whenever a
  SimTask runs longer than the soft limit, a warning is logged. If it runs
  longer than the hard limit, the participant switches to the error state.
- New config section for ParticipantControllers. Here, you can specify the execution time limits for SimTasks and specify the SyncType. E.g.,
  
    .. code-block:: javascript
                    
       "ParticipantController": {
           "SyncType": "DiscreteTime",
           "ExecTimeLimitSoftMs": 1010,
           "ExecTimeLimitHardMs": 1500
       }
   
- New ParticipantControllerBuilder. When using the builder pattern to generate
  an Ib Config, the ParticipantController can be configured via
  ParticipantBuilder::AddParticipantController().

- New Ethernet Trace Logging in the PCAP Format. Trace logs can be written to files
  or named pipes.
- New config parameters are added for EthernetControllers. Here, you can specify
  the PCAP trace filename or the name of the pipe. For example:
  
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "MacAddr": "F6:04:68:71:AA:C2",
               "PcapFile": "EthernetReader.pcap",
               "PcapPipe": "EthernetReaderPipe"
           }
       ]
- Added WithPcapFile and WithPcapPipe methods for ControllerBuilder<EthernetController>.
  When using the builder pattern to generate an Ib Config, PCAP tracing can be configured
  with the new methods.

Changed
~~~~~~~
- Transitions from the shutdown state to the error state are no longer allowed.
  With the recently added connection loss detection, participants could also
  enter the error state after a normal shutdown, which is now prevented.

- Implemented new versioning schema. As of now, the following semantic
  versioning schema is applied:
  
  + major number changes indicate breaks on a network layer
  + minor number changes indicate API of config format breaks
  + patch number changes indicate any other non-breaking changes.

- Added a canId field to the CanTransmitAcknowledge data type. This was required
  for a bug fix and is a breaking change on the network layer.
  
- Added a sourceMac field to the EthTransmitAcknowledge data type. This was
  required for a bug fix and is breaking change on the network layer.
  
Fixed
~~~~~
- CAN controllers now only call the TransmitStatusHandler if they did send the
  corresponding CAN message. Previously, in a simulation with more than two CAN
  controllers, the callback could be triggered without having sent a message.

- Ethernet controllers now only call the MessageAckHandler if they did send the
  corresponding ethernet message. Previously, in a simulation with more than two
  ethernet controllers, the callback could be triggered without having sent a
  message.

Deprecated
~~~~~~~~~~~~~~
- The Participant config setting SyncType has been deprecated. The SyncType is
  now configured in the ParticipantController section.

  + old:
    
    .. code-block:: javascript
                    
       "SyncType": "DiscreteTime"

  + new:
  
    .. code-block:: javascript
                    
       "ParticipantController": {
           "SyncType": "DiscreteTime"
       }

- The SyncType::Unsynchronized is no longer used. Only participants with a
  ParticipantController configuration are synchronized. I.e., you can simple
  remove the "SyncType": "Unsynchronized" from SystemMonitors.
   
- The ParticipantBuilder::WithSyncType() has been deprecated. Use
  ParticipantConfigBuilder::WithSyncType() instead.

  + old:
    
    .. code-block:: c++
                    
      simulationSetup.AddParticipant("P1")
          .WithSyncType(SyncType::DiscreteTime);
    
  + new:
    
    .. code-block:: c++
                    
      simulationSetup.AddParticipant("P1")
          .AddParticipantController().WithSyncType(SyncType::DiscreteTime);


Compatibility with 1.1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Partially (Everything except for CAN and Ethernet is compatible)
- Middleware network protocol (VAsio): Partially (Everything except CAN and Ethernet is compatible)


[1.1.0] - 2019-09-16
--------------------------------
Added
~~~~~
- Added a connection loss mechanism in FastRTPS and VAsio middleware. Lossing the
  connection of one participant will lead the system to go into Error state.
- When logging at trace level, a log entry is now written for each incoming and
  outgoing IbMessage.
- When logging at trace level, the wait times and execution times per tick are
  now logged.
- The creation of a ComAdapter is now logged. The log entry includes the used
  VIB version.
- Connection losses during the simulation are now detected. The are reported as
  an updated ParticipantStatus with State Error and a note that the connection
  was lost.

Fixed
~~~~~
- Fixed FlexRay parameter validation, which could cause valid parameter sets to
  be rejected. E.g., gdSymbolWindows has a valid range from 0 to 162, but we
  check for 1 to 139.
- Fixed a racecondition when starting up a VAsio simulation. As VAsio does not
  have a history, it could happen that some participants did not receive all
  ParticipantStatus values.

.. _changelog:1.0.0_removed:

Removed
~~~~~~~
- IComAdapter::RegisterNewPeerCallback() was removed. This method was only
  intended as an IB-internal helper method and never officially announced as
  part of the public API.
  
Compatibility with 1.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No (cf. :ref:`Removed<changelog:1.0.0_removed>`)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes
  

[1.0.0-QA] - 2019-09-25
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- The logging facilities can now be configured per participant using the IB
  config. E.g., different sinks and log levels can be configured.

Changed
~~~~~~~
- The LIN API was redesigned to provide a clearer and simpler interface. To make
  the transition to the new API as simple as possible, we provided extensive
  documentation on the new API itself including usage examples and information
  about what changed in the new API: :doc:`../api/lin`
- Removed spdlog from the public IB API. Spdlog is still used internally but it
  has been removed from the public API to avoid conflicts with user specific
  spdlog installations.
- The CMake build options BUILD_TESTS and BUILD_DOCS were renamed to
  IB_BUILD_TESTS and IB_BUILD_DOCS.
- The Tools folder has been renamed to Utilities to differentiate it more
  clearly from build tools. For the time being, the IbRegistry is the only
  utility.

Fixed
~~~~~
- Fixed a crash in the IbLauncher when the IbConfig did not specify a
  MiddlewareConfig or an ActiveMiddleware.
- Fixed a crash when creating a ComAdapter with the same participant name as a
  previously destroyed one. The crash originated in spdlog.
- Fixed a crash in the VAsioConnection destructor due to a wrong member order.
- Fixed the Fast-RTPS submodule from v1.7.0 to v1.8.1, which got broken during a
  merge.
- The old, unmaintained CHANGELOG.md is no longer installed. Instead, the
  CHANGELOG.rst is installed in addition to the HTML documentation.


Compatibility with Sprint-31
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (FastRTPS): Partially (Everything except LIN is compatible)
- Middleware network protocol (VAsio): Partially (Everything except LIN is compatible)


[Sprint-31] - 2019-08-14
------------------------
Added
~~~~~
- New VAsio middleware as an alternative to Fast-RTPS, the VAsio middleware was
  specifically developped for the integration bus to provide high performance and
  stability. Cf. :doc:`../configuration/middleware-configuration`.

Changed
~~~~~~~
- Upgrade Fast-RTPS to version v1.8.1. This improves stability on Linux.

Fixed
~~~~~
- Fixed a crash in the IbLauncher when the IbConfig did not specify a
  MiddlewareConfig or an ActiveMiddleware.


Compatibility with Sprint-30
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): No


[Sprint-30] - 2019-07-31
------------------------
Fixed
~~~~~
- Attempting to create a ComAdapter with an empty name will now throw
  a misconfiguration exception with a proper error message.


Interface compatibility with Sprint-29
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-29] - 2019-07-17
------------------------
Added
~~~~~
- It is now possible to reconfigure FlexRay TX-Buffers during the simulation, e.g., to change offset
  and repetition. Cf. :cpp:func:`IFrController::ReconfigureTxBuffer()<ib::sim::fr::IFrController::ReconfigureTxBuffer()>`

Changed
~~~~~~~
- This is the last entry to CHANGELOG.md. From now on, the changelog
  will be maintained in docs/CHANGELOG.rst.
- The IB API Headers are no longer added to every project. Instead, a dedicated
  header project IbApi has been added.
- The IbLauncher now prefers Python 3 if available

Fixed
~~~~~
- Fix logger nullptr bug in SystemMonitor
- Fast-RTPS ComAdapter creation is now thread safe

Interface compatibility with Sprint-28
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-28] - 2019-07-03
-------------------------

Added
~~~~~
- New demo that shows how integration tests can be written for the Vector Integration Bus.

Fixed
~~~~~
- IbLauncher can now be started from every directory location on Linux and Windows. The global
  IntegrationBus-BinPath and IntegrationBus-LibPath are now set to absolute paths inside the
  IbLauncher project. Furthermore, the IbLauncher shell script now sets absolute paths for the bin
  and lib path.
- FastRTPS socket buffer sizes now use default values when not set in IbConfig. This could lead to
  random socket buffer sizes in release builds.
- VIB integration tests now can be launched directly from the Visual Studio test runner.

Interface compatibility with Sprint-27:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-27] - 2019-06-19
------------------------

Changed
~~~~~~~
- NetworkSimulator VIBE is now only used for configured links. For all other links,
  the trivial simulation is used.

Fixed
~~~~~
- The IB Launcher will now work if installed in a path containing spaces.
- The FlexRay configuration will now use strings to represent the enumeration values of pChannels,
  pWakeupChannel, and pdMicrotick, as well as channels and transmissionMode for the TxBuffers. The
  new valid values are:

  * Channels: "A", B", or "AB"
  * pdMicrotick: "12.5ns", "25ns", or "50ns"
  * transmissionMode: "Continuous" or "SingleShot"

Interface compatibility with previous version:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-26] - 2019-05-29
------------------------

Added
~~~~~

- New FlexRay controller callback
  :cpp:type:`IFrController::CycleStartHandler()<ib::sim::fr::IFrController::CycleStartHandler>`,
  which is called at the start of each FlexRay cycle. Only available
  VIBE simulation.
- New config option for FastRTPS middleware to configure SocketBuffer sizes
- New config options to configure FlexRay TxBuffers

Fixed
~~~~~

- Fixed broken CMake target for installed IntegrationBus target:
  With the introduction of spdlog, the IntegrationBus cmake target depends on spdlog::spdlog target.
  However, the spdlog::spdlog target was not installed (only header files were copied, but no cmake
  config file was generated). Thus, the installed IntegrationBus target could not be used from cmake.

[Sprint-25] - 2019-05-14
------------------------

Added
~~~~~

- The FlexRay cluster and node configuration can now be specified in the IbConfig.json.
- It is now possible to use
  :cpp:func:`SetResponse()<ib::sim::lin::ILinController::SetResponse()>`
  and
  :cpp:func:`RequestMessage()<ib::sim::lin::ILinController::RequestMessage()>`
  on LIN :cpp:class:`Controllers<ib::sim::lin::ILinController>` configured as master. This can be used to send a
  LIN message from a master to slaves instead of the
  :cpp:func:`SendMessage()<ib::sim::lin::ILinController::SendMessage>`.

Fixed
~~~~~

- LIN Controller models are now robust to out-of-order configuration. It is no longer necessary to
  configure the master node before the slaves. And the master can handle an out-of-order
  configuration, e.g., if a response reaches the master before the response configuration.


[Sprint-24] - 2019-03-13
------------------------

Added
~~~~~

- Support to swap out participants between simulation runs. A participant can activate the so called
  coldswap feature by calling
  :cpp:func:`IParticipantController::EnableColdswap()<ib::mw::sync::IParticipantController::EnableColdswap()>`.
  The coldswap process can be initiated by a system controller once the system is in state stopped.
- Participants can now signal that they are alive by refreshing the participant status. This can be
  done by calling
  :cpp:func:`IParticipantController::RefreshStatus()<ib::mw::sync::IParticipantController::RefreshStatus()>` and
  is reflected in the new field ParticipantStatus::refreshTime.
- Logging is finally here. We've integrated spdlog and enabled distributed logging with a new spdlog
  sink. The FastRtpsComAdapter automatically creates an spdlogger with this sink. You can access
  this logger via :cpp:func:`IComAdapter::GetLogger()<ib::mw::IComAdapter::GetLogger()>` and add
  any further sinks, e.g., to print logging messages to std out. Examples for this can be found in
  the CAN demo and in the PassiveSystemMonitor.

Changed
~~~~~~~

- The signature of simulation tasks has changed from void(std::chrono::nanoseconds now) to
  void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration). The guaranteed simulation
  time that can be processed is [now, now+duration). The old signature is still available but is now
  considered deprecated and will be removed in a future sprint.


[Sprint-23] - 2019-02-20
------------------------

[Sprint-22] - 2019-02-06
------------------------

Added
~~~~~

- The IbConfig is now validated before creating a ComAdapter. NB: ib::CreateFastRtpsComAdapter()
  will now also throw Misconfiguration exceptions!

Fixed
~~~~~

- The IbLauncher now correctly shows both stdout and stderr. Previously, only stdout was shown.


[Sprint-21] - 2019-01-23
------------------------

Added
~~~~~

- Strict sync, i.e., calling wait_for_all_acked() between ticks, is now
  configurable via the IbConfig.json: SimulationSetup/TimeSync/SyncPolicy.
- If a participant is configured as SyncMaster, the corresponding
  ComAdapter will now automatically create the SyncMaster instance.

Removed
~~~~~~~

- SimulationSetup/TimeSync/SyncType has been removed
  from the IbConfig.json as the SyncType can now be configured per participant.
- IParticipantController::EnableStrictSync() has been removed.
  This is now handled automatically according to the configured SyncPolicy.

Changed
~~~~~~~

- The TimeSyncConfigBuilder is now accessed ib::cfg::SimulationSetupBuilder::ConfigureTimeSync().
  Old: ib::cfg::SimulationSetupBuilder::SetSyncType(SyncType).
- ComAdapter is now configured automatically according to SyncPolicy. Only in
  strict mode, wait_for_all_acked() is used and a short heartbeat period is used.
- IComAdapter::CreateSyncMaster() was renamed to IComAdapter::GetSyncMaster() since the
  SyncMaster is automatically instantiated by the FastRtpsComAdapter if configured.

Fixed
~~~~~

- SystemMonitor was made more robust to race conditions that could lead to a IB
  Startup Failure (SystemState stuck in SystemState::Invalid)


[Sprint-20] - 2018-12-19
------------------------

Added
~~~~~

- LIN: new method ILinController::SetResponseWithChecksum() to override the
  configured checksum model. This can be used to facilitate fault injection and
  simulation.
- LIN: support for LIN network management (sleep / wakeup). See new methods
  ILinController methods: SetSleepMode(), SetOperationalMode(), SendGoToSleep(),
  SendWakeupRequest() and related callbacks.

Removed
~~~~~~~

- Demo projects ExecutionController and ExecutionControllerProxy were based on
  the deprecated synchronization API and were Removed

Changed
~~~~~~~

- LIN: IMPORTANT you must now specify a LIN ID for each lin::SlaveResponseConfig.
  lin::SlaveConfig now longer identifies the lin::SlaveResponses by positions.
- All demo projects Can, Lin, Ethernet, FlexRay, GenericMessage and Io now use
  the new synchronization API.
- Revised public API:

  - Renamed files: IoDataTypes.hpp -> IoDatatypes.hpp
  - Fixed inconsistent naming:

    - IComAdapter.hpp: RegisterCanBusSimulator -> IComAdapter::RegisterCanSimulator
    - CanDatatypes.hpp: Removed 'e' prefix from enum classes CanControllerState, CanErrorState, CanTransmitStatus
    - EthDatatypes.hpp: Removed 'e' prefix from enum classes EthTransmitStatus, EthState, EthMode
    - IEthController.hpp: Renamed 'acticate()', 'deacticate()' -> 'Acticate()', 'Deacticate()'
    - ISyncAdapterTtd.hpp: Renamed SetOnTickCallback -> SetTickHandler
    - IGenericSubscriber.hpp: Renamed RegisterCallback -> SetReceiveMessageHandler
    - ISyncMasterDt.hpp: Renamed RegisterShutdownHandler -> SetShutdownHandler

  - Moved generic messages into subnamespace: ib::sim -> ib::sim::generic
  - Made include namespaces reflect folder names:
    ib::Simulation::Can|Ethernet|Flexray|Generic|Io|Lin|Kernels -> ib::sim::can|eth|fr|generic|io|lin|kernels


[Sprint-19] - 2018-12-05
------------------------

Added
~~~~~

- New unified SyncMaster that replaces the SyncMasterDt and
  SyncMasterTtd. Requires new state handling with ParticipantController
- The participant discovery mechanism can now be configured in the
  IbConfig.json, section "MiddlewareConfig/FastRTPS/DiscoveryType". The default
  is Local, which limits communication to the localhost.
- The build number of the master branches CD build is now available as
  ib::version::BuildNumber()
- On windows, version information is now available as metadata of the
  IntegrationBus.dll

Removed
~~~~~~~

- "MiddlewareConfig/FastRTPS/CommunicationMaster" has been removed
  and replaced with the new DiscoveryType options.

Changed
~~~~~~~

- SyncMasterDt and SyncMasterTtd are now considered deprecated.

Fixed
~~~~~

- Unicast discovery is now working with DiscoveryType Unicast and a list of the
  participants' IP-Addresses.


[Sprint-18] - 2018-11-21
------------------------

Added
~~~~~

- New IbLauncher to startup a whole IB System. See /Launcher/README.md for infos
  on how to use (AFTMAGT-120)
- Version information is now available in the IB library via the following API
  calls: ib::version::Major(), ib::version::Minor(), ib::version::Patch(),
  ib::version::String(), ib::version::SprintNumber(), ib::version::SprintName(),
  ib::version::GitHash() (AFTMAGT-154)

Removed
~~~~~~~

- LinkId has been removed from CAN, LIN, FlexRay and Ethernet data types, as
  they are no longer needed (see below).

Changed
~~~~~~~

- LIN masters now directly store slave responses to answer any request without
  delay. Instead of emulating LIN communication over the IB, LIN slaves now send
  newly configured response data to masters. (AFTMAGT-155) NB: calling
  ILinController::RequestMessage() will now trigger a callback to the registered
  MessageHandler before RequestMessage() returns!
- There is now one FastRTPS topic per link. I.e., if there are two CAN busses
  CAN1 and CAN2 in your configuration, they will now use separate
  topics. Previously, traffic of different links (busses) was separated by a
  linkid field in the message data types and controllers had to filter out
  messages accordingly. This is no longer necessary. (AFTMAGT-140)


[Sprint-17] - 2018-11-07
------------------------

Added
~~~~~

- New state handling, which is provided by the following classes:

-- sync::IParticipantController (cf. IComAdapter::GetParticipantController())
   allows registering callbacks for the different phases of a participant's life
   time (e.g., Initialization, Running, Stop, Shutdown) and replaces the old
   sync::ISyncAdapterTtd and sync::ISyncAdapterDt.

-- sync::ISystemMonitor (cf. IComAdapter::GetSystemMonitor()) is a passive
   component, which never sends data, it allows registering callbacks to observe
   the states of the other participants as well as the global system state.

-- sync::ISystemController (cf. IComAdapter::GetSystemController()) is the
   counterpart to the system monitor and allows manipulating the system state,
   e.g., by initializing individual participants.

-- sync::ISyncMaster (cf. IComAdapter::CreateSyncMaster()) a unified synchronization master
   (currently only supports simple Tick/TickDone synchronization).

-- Participants can now inidividually specify one of the following synchronization mechanisms:

  - DiscreteEvent (not implemented yet)
  - TimeQuantum (Quantum Request and Grant with variable quantum lengths)
  - DiscreteTime (Fixed interval synchronization with participant acknowledgement (Tick/TickDone))
  - DiscreteTimePassive (the participant receives Ticks but does not generate
    TickDone messages and thus does not actively participate in the system
    synchronization), Unsychronized (for participants that are only intended to
    monitor or control the simulation but not participante as an active client)

- GenericMessage configuration is now available at both IGenericPublisher and
  IGenericSubscriber (AFTMAGT-137)

Changed
~~~~~~~

- The old state handling including the old SyncAdapters is now considered deprecated!
  This affects the following classes: sync::ISyncAdapterDt, sync::ISyncAdapterTtd,
  sync::ISyncMasterTtd, sync::ISyncMasterDt, sync::IExecutionControllerProxy.
  And the matching factory methods: IComAdapter::CreateSyncAdapterDt(),
  IComAdapter::CreateSyncAdapterTtd(), IComAdapter::CreateSyncMasterTtd(),
  IComAdapter::CreateSyncMasterDt(), IComAdapter::CreateExecutionControllerProxy().
- The SyncType can now be configured per participant (Currently only DiscreteTime supported)


[Sprint-15] - 2018-10-10
------------------------

Added
~~~~~

- Documentation for throwing behavior at API level (AFTMAGT-50)
- Doxygen documentation for vehicle network controller APIs (AFTMAGT-126)
- Automated CI build system for Jenkins CI, cf. folder /IntegrationBus/ci/ (AFTMAGT-55)
- Support for custom FastRTPS XML configurations; the file name can be
  specified in the ib config (Config.middlewareConfig.fastRtps) (AFTMAGT-138)

Fixed
~~~~~

- Fixed a bug that caused the FastRTPS communication to sporadically hang (AFTMAGT-126)
- The number of links for IO ports is no longer limited (AFTMAGT-134)


[Sprint-14] - 2018-09-27
------------------------

Added
~~~~~

- GenericPublisher::Config() and GenericSubscriber::Config() accessors for corresponding config items. This allows retrieving the name of a Publisher or Subscriber (AFTMAGT-125)
- ib::cfg::Config::ToJsonString() converts an integration bus config to a parsable json string.
- CMake install target for the IntegrationBus
- The generic message demo now uses time synchronization, i.e., an ExecutionController is required to run the demo

Changed
~~~~~~~

- ExecutionController demo now terminates automatically unless started with --waitForKeyPress

Fixed
~~~~~

- The byte order of mac addresses in ethernet frames was fixed


[Sprint-13] - 2018-09-12
------------------------

Added
~~~~~

- Changelog :)
- ib::cfg::ConfigBuilder to create IbConfigs programatically, cf. example in Demo/ConfigBuilder/ConfigBuilderDemo.cpp.
- Support for Continous Integration (CI) with Jenkins, cf. /ci/
- IO ports can now be specified and initialized via the IntegrationBus config.
- Support for multiple GenericMessage instances (specified via IbConfig.json)
- Added new top-level section 'MiddlewareConfig' to JSON file for settings FastRTPS/CommunicationMaster and FastRTPS/ConfigFileName; moved existing configuration tree into top-level section 'SimulationSetup'.
- FastRTPS version bump from v1.5.0 to v1.6.0. FastRTPS is now included as a sub module

Removed
~~~~~~~

- ib::sim::IGenericMessageController()
- ib::mw::IComAdapter::CreateGenericMessageController()

Changed
~~~~~~~

- Moved config headers to /include/ib/cfg to match namespace, i.e.,
  /include/ib/Config.hpp moved to /include/ib/cfg/Config.hpp.
- IoPorts are now type specific with direction. IComAdapter::CreateIoPort() has been replaced with: IComAdapter::CreateAnalog{In,Out}(), IComAdapter::CreateDigital{In,Out}(), IComAdapter::CreatePattern{In,Out}(), IComAdapter::CreatePwm{In,Out}()
- IComAdapter::CreateGenericMessageController() has been replaced by IComAdapter::CreateGenericPublisher() and IComAdapter::CreateGenericSubscriber(), cf. updated demo Demo/GenericMessage/


.. _changelog-outdated-reference:

Outdated References
-------------------

Some references in the changelog may have been outdated by changes made to
the documentation due to the development process.

These references were replaced with a link to this notice.

Please take a look at the documentation of the version of SilKit /
IntegrationBus to which the changelog entry belongs.
