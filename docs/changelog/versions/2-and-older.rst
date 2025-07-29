Version 2.xx and older
======================

Added
~~~~~

- The history length of a publisher can now be overridden in the participant configuration.


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
  about what changed in the new API: :doc:`../../api/services/lin`
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
  stability. Cf. :doc:`../../configuration/middleware-configuration`.

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