VIB Changelog
================================

All notable changes to the IntegrationBus project shall be documented in this file.

The format is based on `Keep a Changelog (http://keepachangelog.com/en/1.0.0/) <http://keepachangelog.com/en/1.0.0/>`_.

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
  simulation runs, cf. :ref:`sec:replay-foreign`.

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
  This ensures a clean CANoe installation when CANoe is slow to shutdown, e.g. when launcher is interrupted
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
- Add documentation for :doc:`Replaying and Tracing<usage/replay>` (AFTMAGT-308).
- Added support for replaying FlexRay messages (AFTMAGT-289).
  Replaying FlexRay requires the use of the VIBE-NetworkSimulator.
  The NetworkSimulator is attached as a replay controller to the replay scheduler, cf. :ref:`sec:replay-architecture` for an overview.
- Added support for replaying FlexRay messages in the VIBE-NetworkSimulator (AFTMAGT-290).
  Please note, that the startup/synchronization sequence is not part of a trace file  and time stamps of the
  replay might deviate from the original traced messages.
  The logical order of messages is kept after the synchronization has been established.
  Refer to section :ref:`sec:replay` for a summary of supported features and limitations.
- Add a :ref:`Replay<sec:cfg-participant-replay>` configuration block to the NetworkSimulator
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
- Setting VAsio TCP flags is now supported from a JSON config file, cf. :ref:`sec:mwcfg-vasio` (AFTMAGT-305).


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
  configuration, see the "ConnectAttempts" field in :ref:`sec:mwcfg-vasio`.
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
  refer to :ref:`sec:cfg-participant-replaying` for details.
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
  For backward compatibility the old configuration syntax is still supported,
  refer to :ref:`sec:cfg-network-simulators`.
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
- Added a message tracing section to :ref:`Participant Configuration<sec:cfg-participant-tracing>`.
  It briefly discusses how to configure the trace sink mechanism and how the 
  :ref:`mdf4tracing` extension is used.
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
  cf. :ref:`sec:mwcfg-vasio`. The corresponding :cpp:class:`RegistryBuilder<ib::cfg::VAsio::RegistryBuilder>`
  also gained the :cpp:func:`ConfigureLogger()<ib::cfg::VAsio::RegistryBuilder::ConfigureLogger>` method to configure
  the logger of the VAsio Registry.
- Added benchmark demo, cf. :ref:`sec:util-benchmark-demo`.

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
  the additional search paths for shared library extensions loaded by the VIB,
  e.g. the :doc:`vibes/vibregistry`.

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
- Documentation for the Participant Controller API: :doc:`api/participantcontroller` (AFTMAGT-206).
- Documentation for the IO Port services: :doc:`api/io` (AFTMAGT-201).
- Documented Generic Messages API: :doc:`api/genericmessage` (AFTMAGT-204).
- Documented the simulation state machine and synchronization types: :doc:`simulation/simulation`
- Added docs for the ComAdapter: :doc:`api/comadapter`
- Added quick start guide: :doc:`usage/quickstart`
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
- Extend the simulation setup documentation, cf. :doc:`../configuration/simulation-setup`

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
  the PCAP trace filename or the name of the pipe. E.g.:
  
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
