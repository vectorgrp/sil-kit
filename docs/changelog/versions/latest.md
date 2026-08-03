# [5.0.7] - 2026-07-31

## Added

- `config`: experimental topic based filtering of log messages per logging sink via `Experimental/EnabledTopics` and `Experimental/DisabledTopics`. Log messages are tagged with a topic that classifies their origin (e.g., `Asio`, `Ethernet`, `Participant`, `User`).
- `docs`: added description of the experimental logging topics, including the list of topic names currently assigned by SIL Kit components.

## Fixed

- `core`: on peer shutdown, the disconnected peer is removed from the pending subscription acknowledges. Otherwise, participants could wait indefinitely for acknowledges that will never arrive.
- `config`: the participant configuration is now validated against the schema. Previously, only included configuration files were validated, the root configuration was not.
- `config`: added missing entries to the participant configuration JSON schema and the schema validation (e.g., `Experimental/Metrics/UpdateInterval`, `RpcClients/RpcChannel`, `RpcServers/RpcChannel`).
- `api`: the public header `silkit/services/logging/string_utils.hpp` no longer includes internal headers.
- `config`: experimental `Experimental.TimeSynchronization.DynamicSimulationStep` to enable dynamic
  simulation step sizes, aligning each simulation step to the minimal step among all synchronized
  participants. Tri-state: `true` requests it for the whole simulation, `false` opts out, and leaving
  it unset follows the network. Off by default.
- `logging`: Fixes a bug where some log messages (e.g., user-level log messages) were passed to fmt and caused exceptions when placeholder characters were present. These log messages are no longer passed to fmt.

## Changed

- `config`: default format of file logging is set to JSON. Set "Format: Simple" in the sink to get previous behaviour.
- `docs`: added description of the logging "Format".
- `docs`: added deprecation warning for Ubuntu 20.04
- `build`: the CMake targets only expose the top-level include directory instead of internal include directories.

## Fixed

- `config`: a mapping that is mistyped as a YAML sequence (e.g. a stray leading `- ` turning `HealthCheck` into a list) is now rejected with a clear error instead of being silently accepted.
- `config`: a configuration containing multiple `---`-separated YAML documents is now rejected with a clear error. A single leading `---` remains valid.
- `orchestration`: setting a simulation step duration of zero now raises a `SilKitError` instead of being silently accepted.
