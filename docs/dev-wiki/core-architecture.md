# Core Architecture

This page connects the public SIL Kit architecture model with the actual repository structure and runtime entry points.
It is meant for developers changing implementation code, not for end-user API onboarding.

## Architecture In One View

At a high level, SIL Kit is built around these ideas:

- applications create participants through the public API
- participants expose communication and orchestration services
- participants discover each other through a registry
- communication between participants is peer-to-peer once connections are established
- lifecycle and virtual time coordination are distributed across participants rather than centralized in one simulation process

SIL Kit is designed around these guiding principles:

- local view for participants
- distributed simulation
- stable API and ABI behavior
- reconfigurability via configuration files

## Conceptual Building Blocks

The public conceptual model is made up of the following runtime components:

- `Registry`: discovery and initial connection brokering
- `Participant`: the local runtime node embedded into an application or utility
- `Services`: the APIs attached to a participant, such as CAN, Ethernet, PubSub, RPC, logging, lifecycle, and time sync
- `System Controller`: defines required participants and influences system-wide lifecycle behavior
- `System Monitor`: observes participant and system state transitions

## API Shape And The Hourglass

Versioning documentation describes the public API shape as an hourglass pattern:

- the `C` API is the versioned compatibility anchor
- the `C++` API in `silkit/...` is a header-only wrapper around the `C` API

This is important when making changes:

- changes in public `C` API contracts have the strongest compatibility impact
- changes in the header-only `C++` API can still be user-visible even when the implementation lives deeper in the repository
- internal implementation code should be kept conceptually below the API boundary, even when the C++ API makes the public surface feel direct

## Main Runtime Flow

The most important runtime path is:

1. User code creates or loads a participant configuration.
2. User code creates a participant.
3. Internal code validates and sanitizes the configuration.
4. A concrete participant implementation is instantiated with a transport backend.
5. The participant joins the SIL Kit simulation and discovers peers through the registry.
6. Services exchange messages over established connections.

In the current codebase, the main entry points are:

- `SilKit/source/CreateParticipantImpl.cpp`
- `SilKit/source/core/participant/CreateParticipantInternal.cpp`
- `SilKit/source/core/participant/CreateParticipantT.hpp`

The flow is currently straightforward:

- `CreateParticipantImpl(...)` calls `Core::CreateParticipantInternal(...)`
- `CreateParticipantInternal(...)` instantiates `Participant<VAsioConnection>`
- `CreateParticipantT(...)` runs configuration validation and constructs the participant object
- the created participant then joins the distributed simulation

This means that participant creation is one of the best entry points when you need to understand how configuration, transport, and service setup come together.

## Layer Mapping To Repository Code

The conceptual architecture maps onto the repository roughly like this.

### Public Surface

- `SilKit/include/`: public headers and API contracts
- `SilKit/source/capi/`: C API implementation layer

This is where API and ABI-sensitive work usually begins.

### Participant Construction And Runtime Shell

- `SilKit/source/core/participant/`

This area is responsible for:

- participant creation
- participant configuration validation and sanitization
- participant object construction
- wiring the participant to the chosen transport backend

Important files include:

- `Participant.hpp`
- `Participant.cpp`
- `ValidateAndSanitizeConfig.*`
- `CreateParticipantInternal.*`
- `CreateParticipantT.*`

### Core Runtime Infrastructure

- `SilKit/source/core/internal/`
- `SilKit/source/core/service/`
- `SilKit/source/core/requests/`
- `SilKit/source/core/vasio/`

These areas cover the non-user-facing runtime machinery.

Broadly:

- `core/service/` handles service discovery and related serialization
- `core/requests/` handles request/reply style internals
- `core/vasio/` contains the transport implementation and registry runtime based on the VAsio backend
- `core/internal/` contains deeper internals shared by the participant runtime

### Services

- `SilKit/source/services/can/`
- `SilKit/source/services/ethernet/`
- `SilKit/source/services/flexray/`
- `SilKit/source/services/lin/`
- `SilKit/source/services/pubsub/`
- `SilKit/source/services/rpc/`
- `SilKit/source/services/logging/`
- `SilKit/source/services/metrics/`
- `SilKit/source/services/orchestration/`

These directories implement the participant-visible services.
If a change affects semantics at the controller or service level, this is often the first place to inspect.

### Experimental And Extension Areas

- `SilKit/source/experimental/`
- `SilKit/source/extensions/`
- `SilKit/source/dashboard/`

These areas layer additional or less stable capabilities on top of the core runtime.

## Discovery And Transport

The registry is mandatory for bringing up a SIL Kit system.
Its conceptual role is discovery and initial connection brokering.

In the implementation:

- registry creation currently goes through `SilKit/source/CreateSilKitRegistryImpl.cpp`
- the concrete registry type is `Core::VAsioRegistry`
- the transport backend lives in `SilKit/source/core/vasio/`

A key detail of the runtime model is:

- the registry establishes connections between participants
- participants then communicate through peer-to-peer connections
- if direct transport is not available, the registry can also be used as a proxy path

When working on connectivity issues, relevant implementation files usually live under:

- `SilKit/source/core/vasio/VAsioConnection.*`
- `SilKit/source/core/vasio/VAsioRegistry.*`
- `SilKit/source/core/vasio/ConnectPeer.*`
- `SilKit/source/core/vasio/ConnectKnownParticipants.*`
- `SilKit/source/core/vasio/RemoteConnectionManager.*`

## Service Discovery

A major architectural theme in SIL Kit is that participants typically do not need hardcoded knowledge of peer instances.
Communication is resolved through service descriptions, matching network names, and topic names.

For implementation work, the central discovery-related area is:

- `SilKit/source/core/service/`

Important responsibilities there include:

- service discovery event handling
- discovery filtering and matching
- serialization of service metadata

This layer is the bridge between transport-level connectivity and service-level communication semantics.

## Orchestration And Time

The orchestration subsystem is one of the most important architectural slices because it controls how participants behave as a coherent simulation.

Its implementation is in:

- `SilKit/source/services/orchestration/`

That directory contains the core orchestration pieces:

- lifecycle management and lifecycle state handling
- `LifecycleService`
- `TimeSyncService`
- `SystemController`
- `SystemMonitor`
- system state tracking and supporting serialization/time helpers

Conceptually:

- `LifecycleService` controls a participant's local lifecycle behavior
- `TimeSyncService` adds virtual time synchronization for coordinated simulation steps
- `SystemMonitor` observes participant and system-wide state transitions
- `SystemController` defines required participants and can influence system-wide orchestration

The lifecycle state machine and system state rules matter here mainly as implementation boundaries and code ownership hints.

## Communication Services

The communication-service architecture follows a repeated pattern:

- a participant creates a specific service or controller
- the service announces itself through discovery
- matching peers exchange messages using common service metadata such as network or topic names
- transport and serialization layers carry the actual messages

This pattern is reused across:

- bus-oriented services such as CAN, Ethernet, LIN, and FlexRay
- application-level services such as PubSub and RPC

If a feature spans multiple protocol families, it often means the real architectural concern is below the individual service directories, in discovery, transport, or orchestration.

## Configuration As An Architectural Boundary

Configuration is not just convenience glue in SIL Kit. It is part of the architecture.

Why it matters:

- participants are expected to run without a config file, but they should allow user-provided configuration
- runtime behavior such as logging, middleware setup, and service wiring can be changed without recompilation
- participant construction validates and sanitizes configuration before the runtime is created

Relevant implementation areas include:

- `SilKit/source/config/`
- `SilKit/source/core/participant/ValidateAndSanitizeConfig.*`

When a change appears to be purely local but has configuration impact, check both the config model and the participant-construction path.

## Where To Start For Common Change Types

If you need to change a public API:

- start in `SilKit/include/`
- inspect `SilKit/source/capi/`
- review compatibility implications in version-related code and changelog metadata

If you need to change participant bring-up:

- start in `SilKit/source/CreateParticipantImpl.cpp`
- then inspect `SilKit/source/core/participant/`

If you need to change connectivity or registry behavior:

- start in `SilKit/source/core/vasio/`
- inspect transport and utility implementation paths nearby

If you need to change lifecycle, monitor, or virtual time behavior:

- start in `SilKit/source/services/orchestration/`
- inspect adjacent orchestration code paths and tests

If you need to change protocol-specific messaging behavior:

- start in the relevant `SilKit/source/services/<protocol>/` directory
- then inspect discovery and transport code if the problem spans multiple services

## Architecture Boundaries To Preserve

When changing code, these boundaries are especially worth preserving:

- public API contracts should remain cleanly separated from runtime internals
- service semantics should not accidentally leak transport-specific assumptions upward
- protocol-specific fixes should not duplicate logic that belongs in shared discovery or orchestration layers
- configuration validation should happen before runtime behavior depends on it
- lifecycle and time-sync behavior should remain consistent with the documented state model

## Related Pages

- [Repository Layout](./repository-layout.md)
- [Build and Test](./build-and-test.md)
- [Developer Wiki Front Page](./README.md)
