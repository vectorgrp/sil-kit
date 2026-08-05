# Utilities and Processes

This page explains the built-in SIL Kit utility executables, what role each one plays in a running system, and how their implementations relate back to the library runtime.

## Overview

The repository ships three main utility processes under `Utilities/`:

- `sil-kit-registry`
- `sil-kit-system-controller`
- `sil-kit-monitor`

They are all built when `SILKIT_BUILD_UTILITIES=ON`.

At a high level:

- the registry is required for discovery
- the system controller is used for coordinated simulation startup and system-wide control
- the monitor is optional and used to observe participant and system state

One important distinction is that the registry is mandatory, while the other two utilities are reference applications built on top of the library.

## Build Structure

The top-level utility aggregation is simple:

- `Utilities/CMakeLists.txt` adds `SilKitMonitor`
- `Utilities/CMakeLists.txt` adds `SilKitRegistry`
- `Utilities/CMakeLists.txt` adds `SilKitSystemController`

The three executables are not all built the same way.

### `sil-kit-registry`

The registry executable links mostly against internal/static library pieces:

- `I_SilKit`
- `S_SilKitImpl`
- `O_SilKitRegistry_Config`
- `O_SilKit_Dashboard`

This reflects that the registry is not just a regular participant application. It hosts the concrete registry runtime directly.

### `sil-kit-system-controller`

The system controller links as an application on top of the main library:

- `SilKit`
- `I_SilKit`
- `O_SilKit_Util_SignalHandler`

This utility behaves much more like a normal SIL Kit participant.

### `sil-kit-monitor`

The monitor is also a participant-style executable and links similarly:

- `SilKit`
- `I_SilKit`
- `O_SilKit_Util_SignalHandler`

This is a useful rule of thumb:

- registry: infrastructure process with deeper internal linkage
- controller and monitor: participant-based tools built on public library behavior

## `sil-kit-registry`

### Purpose

The registry is the mandatory process used for participant discovery.
Participants must be able to connect to it in order to join a SIL Kit system.

Conceptually, the registry:

- listens for participant connections
- establishes the initial discovery graph
- provides already-known participant information to newly joining participants
- may also act as a proxy fallback for participant-to-participant communication when direct connectivity is not possible

The normal steady-state goal is still peer-to-peer participant communication.

### Source Location

- `Utilities/SilKitRegistry/`

Important files:

- `Registry.cpp`
- `Registry.hpp`
- `config/RegistryConfiguration.*`
- `WindowsServiceMain.*`
- `ExampleRegistryConfiguration.yaml`

### Implementation Shape

`Registry.cpp` handles:

- command-line parsing
- optional registry configuration file loading
- logging setup
- optional dashboard hookup
- configuration sanitization
- creation of the concrete registry object
- startup via `StartListening(...)`
- optional generated configuration-file output
- signal-driven shutdown handling

The concrete runtime object is created through the internal registry creation path described in the architecture page.

### Configuration Behavior

The registry accepts both command-line parameters and a registry configuration file.
The code explicitly supports overriding command-line defaults from the configuration file.

Important behaviors worth remembering:

- the effective listen URI may be changed by sanitization and startup
- if port `0` is used, the chosen runtime port can be written back into a generated config file
- the registry can be started with dashboard integration
- on Windows service runs, domain sockets are disabled in the registry configuration path

This utility is the right place to look when the question is "why is the registry actually listening here?" rather than "why can't participants connect?"

### Windows Service Support

The registry has platform-specific support for running as a Windows service.

That support lives in:

- `WindowsServiceMain.cpp`
- `WindowsServiceMain.hpp`

This is specific to the registry utility, not a general property of all utilities.

## `sil-kit-system-controller`

### Purpose

The system controller is the process that defines which participants are required for a coordinated simulation and can issue system-wide control actions such as stop or abort.

Conceptually, it exists to answer these questions:

- which participants are required before the coordinated simulation can start?
- when should the workflow configuration be published?
- how should stop or abort be triggered from a central control point?

### Source Location

- `Utilities/SilKitSystemController/`

Primary file:

- `SystemController.cpp`

### Implementation Shape

The implementation is a participant-based application.

In broad strokes it does the following:

1. creates a SIL Kit participant
2. creates an experimental system controller service from that participant
3. creates a system monitor
4. observes participant connect and disconnect events
5. waits until all required participants are connected
6. publishes workflow configuration including the required participant set
7. starts a coordinated lifecycle for the controller participant itself
8. handles stop or abort logic on user signal or system-state changes

The core controller class in the file wires together:

- `CreateSystemController(...)`
- `CreateSystemMonitor()`
- `CreateLifecycleService(...)`

This means the executable is a good example of how orchestration services compose in a real participant.

### Why It Uses The Monitor

The system controller is not implemented as a blind command sender.
It also observes the system through `ISystemMonitor` callbacks.

That is how it:

- tracks which required participants are already connected
- logs remaining required participants
- reacts to stopping or error system states

This design is useful to remember when changing orchestration behavior: system control and system observation are intentionally coupled in this utility.

### Stop And Abort Semantics

The controller handles shutdown pragmatically:

- if the system is running or paused, it issues `Stop(...)`
- if the system is already aborting, it only logs that situation
- otherwise it attempts `AbortSimulation()`

It also retries while waiting for final shutdown and escalates from stop to abort if needed.

This is implementation logic in the utility, not the only possible orchestration policy.
If product behavior questions arise, separate:

- library-level orchestration semantics
- policy choices made by this specific reference executable

## `sil-kit-monitor`

### Purpose

The monitor is an observer utility for participant and system state.
It is optional and can be started or restarted independently.

Conceptually, it is the simplest of the three tools:

- connect to the registry
- observe who is present
- observe participant state transitions
- observe overall system state transitions
- optionally run with lifecycle and time-sync behavior of its own

### Source Location

- `Utilities/SilKitMonitor/`

Primary file:

- `PassiveSystemMonitor.cpp`

### Implementation Shape

The monitor is again a participant-style executable.

Its base behavior is:

1. load or synthesize a participant configuration
2. create a participant
3. create a system monitor from that participant
4. register logging callbacks for connection, participant status, and system state changes
5. optionally create a lifecycle service and time sync service, depending on command-line mode
6. wait for signal-based shutdown

The code shows a useful distinction:

- the utility can be purely observational without owning a lifecycle
- or it can join with autonomous/coordinated lifecycle behavior
- or it can also participate in virtual time if `--sync` is selected

That makes it both a monitoring tool and a compact example program for orchestration APIs.

### Why It Is Described As Passive

The documentation calls the monitor a passive participant because its main purpose is observation rather than control.
In practice, the code still creates a real participant and can optionally have lifecycle participation.

So "passive" here should be read as operational intent, not as "not a participant".

## How The Three Utilities Relate

These utilities form a useful mental stack:

- `sil-kit-registry`: transport/discovery infrastructure
- `sil-kit-system-controller`: orchestration policy and system-wide control
- `sil-kit-monitor`: orchestration/system-state observation

Or in another form:

- registry answers "how do participants find each other?"
- system controller answers "when is the coordinated system allowed to run and how is it stopped?"
- monitor answers "what is the system doing right now?"

Only the registry is mandatory for basic operation.
The other two are convenience and reference implementations built on top of the same underlying services available to any SIL Kit participant.

## Where Logic Lives: Utility vs Library

When working on these tools, it helps to distinguish utility logic from reusable library behavior.

Mostly library behavior:

- registry transport runtime
- participant creation
- lifecycle semantics
- system controller service semantics
- system monitor service semantics

Mostly utility behavior:

- command-line parsing
- logging and console output choices
- exact stop/abort policy used by the controller executable
- generated config-file handling in the registry executable
- how the monitor formats and emits observations

If you are fixing a bug, ask first whether it belongs in:

- the reusable runtime or service implementation, or
- only in the executable's policy and presentation layer

## When To Read Which Utility

Read `SilKitRegistry/` when:

- participants cannot discover each other
- listen URI or generated configuration behavior looks wrong
- dashboard hookup or Windows service behavior is involved

Read `SilKitSystemController/` when:

- coordinated startup does not begin as expected
- required participant handling is wrong
- stop or abort interactions are surprising

Read `SilKitMonitor/` when:

- participant or system state observation is unclear
- you need a minimal example of monitor callbacks or optional lifecycle usage

## Related Pages

- [Core Architecture](./core-architecture.md)
- [Networking and Transport](./networking-and-transport.md)
- [Repository Layout](./repository-layout.md)
- [Developer Wiki Front Page](./README.md)
