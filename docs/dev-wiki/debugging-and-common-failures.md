# Debugging and Common Failures

This page summarizes the failure modes that are most likely to matter during day-to-day development in the SIL Kit repository.

It is organized around the layers where problems usually surface:

- configuration loading
- participant startup
- registry connectivity
- participant-to-participant connectivity
- lifecycle and orchestration
- version and interoperability mismatches
- local build and test workflow

## General Approach

When debugging SIL Kit issues, it usually helps to separate the problem into one of four buckets first:

1. build/configuration problem before the process starts
2. participant startup problem while joining the simulation
3. transport/discovery problem after connecting to the registry
4. lifecycle or service-level problem after communication has started

Many confusing symptoms come from misclassifying a transport problem as a service problem, or a lifecycle problem as a transport problem.

## First Questions To Ask

Before diving into code, answer these quickly:

1. Did the participant process start and load its configuration successfully?
2. Did it connect to the registry?
3. Did it finish connecting to all known participants?
4. Is the issue only visible in coordinated mode or only with time synchronization enabled?
5. Is the issue reproducible in a narrow local setup with one registry and two participants?

Those five questions usually cut the search space down faster than reading long logs linearly.

## Configuration Failures

### Typical Symptoms

- `Error: Failed to load configuration ...`
- `Unknown schema version '...' found in participant configuration!`
- `Unknown schema version '...' found in registry configuration!`
- startup falls back to defaults unexpectedly

### Common Causes

- malformed YAML or JSON
- wrong schema version in the configuration file
- confusion between command-line values and configured values
- unexpected includes or include search paths in participant configuration handling
- empty or mismatched participant name assumptions

### Relevant Code Paths

- `SilKit/source/config/ParticipantConfigurationFromXImpl.cpp`
- `SilKit/source/core/participant/ValidateAndSanitizeConfig.cpp`
- `Utilities/SilKitRegistry/config/RegistryConfiguration.cpp`

### Useful Mental Model

There are two different stages here:

- deserialization and schema validation
- sanitization and default/override resolution

If the process fails before creating a participant, the problem is usually in deserialization.
If it starts but behaves differently than expected, the problem is often in sanitization or override precedence.

### Practical Checks

- confirm the participant name is not empty
- confirm the registry URI being used is the one you intended
- confirm whether the config file overrides the command-line arguments you passed
- for registry config, confirm the schema version matches the parser expectation

## Participant Startup Failures

### Typical Symptoms

- `JoinSimulation: no acceptors available`
- `Something went wrong: ...` very early in startup
- no service callbacks ever fire

### What It Usually Means

The process reached participant construction but failed before or during join.

The most important startup path is:

- participant configuration is sanitized
- acceptors are opened
- the registry connection is attempted
- the registry handshake completes
- known participants are contacted

If startup dies before the registry handshake, look at participant construction and transport setup rather than service logic.

### Relevant Code Paths

- `SilKit/source/CreateParticipantImpl.cpp`
- `SilKit/source/core/participant/CreateParticipantInternal.cpp`
- `SilKit/source/core/participant/CreateParticipantT.hpp`
- `SilKit/source/core/vasio/VAsioConnection.cpp`

### Common Causes

- no valid local acceptor endpoints could be opened
- invalid local-domain socket assumptions
- network binding problems on the machine
- conflicting participant names leading to handshake failure later in startup

## Registry Connectivity Failures

### Typical Symptoms

- `Failed to connect to SIL Kit Registry`
- repeated messages about connection attempts
- startup never reaches peer connection setup

### What It Usually Means

The participant cannot establish the first required connection in the system.
This is a registry reachability problem, not a service problem.

### Relevant Code Paths

- `SilKit/source/core/vasio/VAsioConnection.cpp`
- `Utilities/SilKitRegistry/Registry.cpp`
- `Utilities/SilKitRegistry/config/RegistryConfiguration.cpp`

### Common Causes

- wrong connect URI or listen URI
- hostname not resolvable from the participant host
- registry only reachable through local domain sockets in the current environment
- firewall, container, VM, WSL, NAT, or host-interface mismatch
- registry bound to an interface that remote participants cannot reach

### Practical Checks

- verify which registry URI the participant is actually using after sanitization
- verify which URI the registry is actually listening on at runtime
- check whether `localhost` is meaningful in the current topology
- check whether domain sockets are helping or hurting in the current environment

### Key Diagnostic Distinction

If the participant never reports a successful registry connection, do not debug controller, PubSub, CAN, or lifecycle behavior yet.
Those layers are downstream of registry reachability.

## Participant-To-Participant Connectivity Failures

### Typical Symptoms

- registry connection succeeds, but startup still fails
- `Failed to connect to known participants: ...`
- `Timeout while waiting for replies from known participants: ...`
- `Timeout during connection setup. The participant was able to connect to the registry, but not to all participants.`
- proxy fallback warnings appear

### What It Usually Means

Discovery worked, but the direct peer graph could not be completed.

This is one of the most common real-world categories of failures.

### Relevant Code Paths

- `SilKit/source/core/vasio/ConnectKnownParticipants.cpp`
- `SilKit/source/core/vasio/ConnectPeer.*`
- `SilKit/source/core/vasio/RemoteConnectionManager.cpp`
- `SilKit/source/core/vasio/TransformAcceptorUris.cpp`
- `SilKit/source/core/vasio/VAsioConnection.cpp`

### Common Causes

- advertised endpoints are not reachable from the other host
- loopback or catch-all addresses are wrong for the topology
- domain sockets are advertised in a situation where only TCP is usable
- remote connect request capability is not enabled or not supported by the peer
- proxy fallback capability is disabled locally or remotely

### How To Think About It

For each discovered participant, the runtime tries:

1. direct connect
2. remote connect request
3. registry proxy fallback

If all three fail, the join fails.

So the debugging questions are:

1. Was the direct endpoint actually reachable?
2. Was reverse connection even possible by capability?
3. Was proxy fallback allowed by both sides?

### Proxy Fallback Warnings

If the logs say direct connection failed and the registry is being used as a proxy, the system may still function.

That means:

- the problem is real
- the setup is degraded rather than fully broken
- latency and overhead will usually be worse

Treat this as a topology/debugging issue, not as a normal steady-state success case.

## Duplicate Participant Name And Handshake Failures

### Typical Symptoms

- `Timeout during connection handshake with the SIL Kit Registry`
- message suggesting that a participant with the same name may already be connected
- failed participant announcement reply diagnostics

### What It Usually Means

Two participants are trying to join the same registry with the same participant name, or there is a version-mixed handshake edge case around that situation.

### Relevant Code Paths

- `SilKit/source/core/vasio/VAsioConnection.cpp`
- registry-side participant announcement handling

### Practical Checks

- confirm every participant name is unique in the running system
- check whether an older crashed process is still connected
- if mixed versions are involved, do not assume the error text is perfectly modern or perfectly specific

Duplicate-name failures are often misread as generic transport issues because they surface during handshake timeouts.

## Lifecycle And Orchestration Failures

### Typical Symptoms

- coordinated participants never start running
- system controller waits forever for required participants
- simulation enters error state unexpectedly
- stop and abort behavior feels inconsistent
- messages like `This participant is in OperationMode::Coordinated, but is not among the participants that are reported to the system controller as "required".`
- `Required participant names are already set.`
- `Tried to instantiate ... multiple times`

### What It Usually Means

Transport may already be healthy.
The problem is now in orchestration policy, participant mode, or service-instantiation semantics.

### Relevant Code Paths

- `SilKit/source/services/orchestration/`
- `Utilities/SilKitSystemController/SystemController.cpp`
- `Utilities/SilKitMonitor/PassiveSystemMonitor.cpp`

### Common Causes

- coordinated participants are not listed as required participants
- more than one actor tries to define workflow configuration
- lifecycle, time sync, or system monitor are instantiated multiple times on the same participant
- shutdown is being attempted from a system state where stop is not sufficient and abort is required

### Useful Distinctions

- transport problem: cannot connect to peers
- orchestration problem: can connect, but never reaches the expected lifecycle state

If the registry and peer graph are healthy, and the system still does not run, move into lifecycle debugging quickly.

### Practical Checks

- confirm whether the participant is autonomous or coordinated
- confirm whether coordinated participants are in the required set
- confirm that workflow configuration is only being set once
- confirm that singleton-style services are not being instantiated multiple times

## Time Synchronization Confusion

### Typical Symptoms

- simulation appears frozen in coordinated mode
- simulation step handler never fires
- callbacks fire in an order different from what was expected

### What It Usually Means

The participant may have a lifecycle but has not reached the state where synchronized time can advance.

Common causes include:

- coordinated participants still waiting for missing required peers
- time sync service not created or not configured as expected
- confusion between communication-ready, ready-to-run, and running phases

This is usually not a transport problem if peer connectivity is already complete.

## Interoperability And Version Mismatch Problems

### Typical Symptoms

- `Network incompatibility between this version range ...`
- participant announcement reply version errors
- misleading handshake diagnostics in mixed-version environments

### What It Usually Means

The system crossed a compatibility boundary in API-independent runtime behavior, usually at the protocol or handshake level.

### Relevant Code Paths

- `SilKit/source/core/vasio/VAsioConnection.cpp`
- version and registry message handling nearby

### Common Causes

- genuinely incompatible protocol behavior
- mixed old/new registry and participant combinations
- duplicate participant names in older-version combinations producing less clear diagnostics

### Practical Checks

- confirm whether all participants and utilities are from a compatible version line
- confirm whether the error is a true protocol mismatch or a duplicate-name edge case
- if the failure mentions handshake protocol version, inspect both participant and registry versions together

## Build And Test Failures During Development

### Typical Symptoms

- build config succeeds but tests are missing
- `ctest` runs fewer suites than expected
- a utility or demo binary is missing from the build tree
- docs target is unavailable

### Common Causes

- wrong preset or manual configuration flags
- `SILKIT_BUILD_TESTS`, `SILKIT_BUILD_UTILITIES`, `SILKIT_BUILD_DEMOS`, or `SILKIT_BUILD_DOCS` not enabled as expected
- filtering `ctest` too aggressively
- expecting demos or docs in a build configuration that does not include them

### Practical Checks

- inspect the chosen preset in `CMakePresets.json`
- inspect the root `CMakeLists.txt` options
- use `ctest -N` to confirm whether tests were registered
- confirm the build tree you are invoking matches the configure preset you think you used

## Fast Debugging Workflow

When you do not know where the problem is, this sequence is usually efficient:

1. Confirm configuration loaded successfully.
2. Confirm participant startup reached registry connection.
3. Confirm registry connection succeeded.
4. Confirm all known participant handshakes completed.
5. Confirm the issue is transport, orchestration, or service-level.
6. Only then move into protocol-specific debugging.

That order avoids spending time in the wrong layer.

## Useful Files By Failure Type

For configuration problems:

- `SilKit/source/config/ParticipantConfigurationFromXImpl.cpp`
- `SilKit/source/core/participant/ValidateAndSanitizeConfig.cpp`

For startup and connectivity problems:

- `SilKit/source/core/vasio/VAsioConnection.cpp`
- `SilKit/source/core/vasio/ConnectKnownParticipants.cpp`
- `SilKit/source/core/vasio/RemoteConnectionManager.cpp`
- `SilKit/source/core/vasio/TransformAcceptorUris.cpp`

For lifecycle and shutdown behavior:

- `SilKit/source/services/orchestration/`
- `Utilities/SilKitSystemController/SystemController.cpp`

For monitor-side observation:

- `Utilities/SilKitMonitor/PassiveSystemMonitor.cpp`

## Related Pages

- [Build and Test](./build-and-test.md)
- [Core Architecture](./core-architecture.md)
- [Networking and Transport](./networking-and-transport.md)
- [Utilities and Processes](./utilities-and-processes.md)
- [Release and Versioning](./release-and-versioning.md)
- [Developer Wiki Front Page](./README.md)
