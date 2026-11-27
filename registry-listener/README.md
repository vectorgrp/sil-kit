# Registry listener proof of concept

A standalone C++17 executable that embeds SIL Kit's registry and prints every
`IRegistryEventListener` callback as a JSON event to stdout. It replaces the registry executable,
not the registry protocol. Existing participants connect normally.

## Dependencies and build

This project consumes a **source checkout**, not the installed SDK. It requires
this checkout's internal listener interface, including the generation lifecycle callbacks.
Private headers and targets are not a supported extension API; pin/revalidate
the revision when using this elsewhere.

The only direct SIL Kit link dependencies are:

- `I_SilKit`: internal include paths and third-party usage requirements.
- `S_SilKitImpl`: the static implementation, including the registry factory,
  configuration loader, event types, and signal helper.

No dashboard client, HTTP service, shared SIL Kit library, or upstream utility
executable is needed. The factory's `WithDashboard` name does not require the
dashboard implementation.

From the SIL Kit repository root:

```sh
cmake -S registry-listener -B build-registry-listener -DCMAKE_BUILD_TYPE=Debug
cmake --build build-registry-listener --target registry-listener -j 4
./build-registry-listener/registry-listener --listen-uri silkit://localhost:8500
```

On this development machine, first source the compiler setup:

```sh
source /u/ccmaster/SDR/main/linux/cde/etc/setup
```

Use the matching compiler's `lib64` directory in `LD_LIBRARY_PATH` when running
if it is not already provided by that setup.

The folder can also be copied outside this repository:

```sh
cmake -S /path/to/registry-listener -B /path/to/poc-build \
    -DSILKIT_SOURCE_DIR=/path/to/sil-kit -DCMAKE_BUILD_TYPE=Debug
cmake --build /path/to/poc-build --target registry-listener -j 4
```

SIL Kit is added in its own build subdirectory. Its tests, demos, utilities,
documentation, and dashboard client are disabled in this independent build.
The parent repository's CMake files and existing build directories are untouched.

## Running and configuration

`--help` lists the options. Stop with SIGINT (Ctrl+C) or SIGTERM.
Port zero selects a free port; `registry_uri` reports the actual bound URI:

```json
{"source":"silkit_registry","version":1,"type":"logger_created"}
{"source":"silkit_registry","version":1,"type":"registry_uri","uri":"silkit://localhost:8500"}
{"source":"silkit_registry","version":1,"type":"participant_connected","simulation":"","name":"Example","generation":null}
```

For example, point an existing demo at the URI printed by the listener:

```sh
./build/Debug/SilKitDemoCanWriter --registry-uri silkit://localhost:8500
```

`--configuration <file>` accepts **participant** configuration YAML/JSON, not the
registry utility's separate configuration schema. The CLI listen URI overrides
`Middleware.RegistryUri`; otherwise that field overrides the default.
Remote metric collection defaults to enabled unless explicitly configured false.
Participant metric producers still need a remote sink:

```yaml
Experimental:
  Metrics:
    Sinks:
      - Type: Remote
```

To disable domain-socket fallback, set `Middleware.EnableDomainSockets: false`.
Configured SIL Kit stdout logging may appear alongside JSON events. Use a file
logging sink if stdout must contain events only. Executable errors go to stderr.

## JSON event contract (version 1)

Each listener event is one complete, flushed JSON object followed by a newline.
Every event has `source: "silkit_registry"`, integer `version: 1`, and a `type`.
A consumer can parse complete lines and select objects by `source` and supported
`version`, ignoring unrelated logging. Malformed lines identifying themselves
as registry events should be reported as stream errors. Field order is not
significant; consumers should ignore unknown fields and event types.

| `type` | Additional fields |
| --- | --- |
| `logger_created` | None; the internal logger address is not exposed. |
| `registry_uri` | `uri` string. |
| `generation_created` | `simulation` string, `generation` unsigned 64-bit integer. |
| `generation_state_changed` | `simulation`, `generation`, `state` string, e.g. `"Running"` or `"Shutdown"`. |
| `generation_removed` | `simulation`, `generation`. |
| `participant_connected`, `participant_disconnected` | `simulation`, participant `name` string, `generation` integer or null. |
| `required_participants` | `simulation`, sender `name`, `generation`, `required` array of participant-name strings. |
| `participant_status` | `simulation`, sender `name`, `generation`, `state`, `enter_reason` string, `enter_time_ns` and `refresh_time_ns` signed integers. |
| `service_discovery` | `simulation`, sender `name`, numeric `event`, structured `service` object. |
| `metrics` | `simulation`, `origin` string, `metrics` array. |

Strings preserve UTF-8, quotes, backslashes, and all control characters through
JSON escaping. Empty strings, arrays, and objects retain their respective types.
Status timestamps are nanoseconds since the system-clock epoch.

`service_discovery.event` is the SIL Kit discovery enum: 0 = Invalid,
1 = ServiceCreated, 2 = ServiceRemoved. Its `service` object contains `id`,
`name`, `participant_name`, `participant_id`, `type`, `network_name`,
`network_type`, and a string-to-string `supplemental` object. `type` and
`network_type` are numeric SIL Kit enum values.

Each metric contains `timestamp_ns`, `name`, numeric `kind`, and string `value`.
Kinds are 0 = COUNTER, 1 = STATISTIC, 2 = STRING_LIST, 3 = ATTRIBUTE.
`timestamp_ns` preserves the producer's nanosecond timestamp. `value` preserves
SIL Kit's original string representation, including JSON-formatted values for
non-attribute metrics; it is not embedded as raw JSON or coerced to a number.
Use integer-aware decoders for 64-bit IDs, generation numbers, and timestamps.

Participant connection, disconnection, status, and workflow events carry the
generation assigned to the sending peer by the registry. This remains
unambiguous when the same participant name exists in concurrent generations.
Legacy simulations have no assigned generation and emit `"generation":null`,
not zero. The new field is additive within version 1.
Discovery and metric events still omit generation identity.
Workflow events report the received configuration, not confirmation that the
placement accepted it. Adapting csil remains a separate change.

### Generation lifecycle

For simulation names starting with `session/`, the first participant in each
new generation also produces a notification before its connection notification:

```json
{"source":"silkit_registry","version":1,"type":"generation_created","simulation":"session/example","generation":1}
{"source":"silkit_registry","version":1,"type":"participant_connected","simulation":"session/example","name":"Example","generation":1}
```

Joining an existing generation or initializing its lifecycle state does not
repeat the event. Recreating an emptied generation (including reusing its
number) does. Legacy simulation names do not produce generation notifications.

Generation lifecycle changes are reported separately:

```json
{"source":"silkit_registry","version":1,"type":"generation_state_changed","simulation":"session/example","generation":1,"state":"Running"}
{"source":"silkit_registry","version":1,"type":"generation_state_changed","simulation":"session/example","generation":1,"state":"Shutdown"}
{"source":"silkit_registry","version":1,"type":"participant_disconnected","simulation":"session/example","name":"Example","generation":1}
{"source":"silkit_registry","version":1,"type":"generation_removed","simulation":"session/example","generation":1}
```

`generation_state_changed` reports changes to the generation's aggregate SIL Kit
system state, not individual participant states. Accepted workflow updates,
participant status updates, and departures can change it. Repeated states and
rejected workflows do not produce state-change events. Notifications continue
after the generation starts, including its shutdown lifecycle.

`generation_removed` follows the last participant's disconnection, even when
no workflow or participant status was reported. Removal discards the tracker;
it does not synthesize a final state or imply that `Shutdown` was reached.
Registry process teardown does not guarantee per-generation removal events.

## Listener behavior and limitations

- All eleven callbacks are printed, including generation lifecycle, required participants, discovery
  supplemental data, and the complete metrics update as structured JSON.
  The former diagnostic-text output format is no longer emitted.
- Logger and URI callbacks run during construction/startup; the rest run on the
  registry I/O thread. No callback payload or logger pointer is retained.
- Dumping is synchronous and flushes each event. Slow stdout can stall the
  registry. An output/formatting failure is reported to stderr and terminates
  the process instead of throwing through the I/O thread.
- Registry destruction precedes listener destruction, including on exceptions.
  The upstream signal helper runs the shutdown notification on a dedicated
  thread, not inside the operating system signal handler.
- This is observational: it does not add admission control or placement changes.

## Smoke test

The optional smoke test needs Python 3 and builds a small participant fixture.
It checks every callback, concurrent generation lifecycles, two simulation names,
disconnection, signal shutdown with a participant connected, startup errors,
and a TCP port conflict. The participant fixture's `--emit-events` mode also
checks JSON round-tripping of control characters and UTF-8, empty collections,
64-bit integers, structured payloads, and locale-independent numeric output.

```sh
cmake -S registry-listener -B build-registry-listener \
    -DCMAKE_BUILD_TYPE=Debug -DREGISTRY_LISTENER_BUILD_TESTS=ON
cmake --build build-registry-listener --target registry-listener registry-listener-test-participant -j 4
ctest --test-dir build-registry-listener -R registry-listener-smoke --output-on-failure
```
