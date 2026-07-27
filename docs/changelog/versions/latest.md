# [5.0.7] - UNRELEASED

## Added

- `config`: experimental topic based filtering of log messages per logging sink via `Experimental/EnabledTopics` and `Experimental/DisabledTopics`. Log messages are tagged with a topic that classifies their origin (e.g., `Asio`, `Ethernet`, `Participant`, `User`).
- `docs`: added description of the experimental logging topics, including the list of topic names currently assigned by SIL Kit components.

## Fixed

- `core`: on peer shutdown, the disconnected peer is removed from the pending subscription acknowledges. Otherwise, participants could wait indefinitely for acknowledges that will never arrive.
- `config`: the participant configuration is now validated against the schema. Previously, only included configuration files were validated, the root configuration was not.
- `config`: added missing entries to the participant configuration JSON schema and the schema validation (e.g., `Experimental/Metrics/UpdateInterval`, `RpcClients/RpcChannel`, `RpcServers/RpcChannel`).
- `api`: the public header `silkit/services/logging/string_utils.hpp` no longer includes internal headers.

## Changed

- `config`: default format of file logging is set to JSON. Set "Format: Simple" in the sink to get previous behaviour.
- `docs`: added description of the logging "Format".
- `build`: the CMake targets only expose the top-level include directory instead of internal include directories.
