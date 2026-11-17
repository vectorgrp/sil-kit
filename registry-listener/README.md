# Registry listener proof of concept

A standalone C++17 executable that embeds SIL Kit's registry and prints every
`IRegistryEventListener` callback to stdout. It replaces the registry executable,
not the registry protocol. Existing participants connect normally.

## Dependencies and build

This project consumes a **source checkout**, not the installed SDK. It requires
this checkout's internal listener interface, including `OnGenerationCreated`.
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
Port zero selects a free port; `OnRegistryUri` prints the actual bound URI:

```text
OnLoggerCreated logger=0x...
OnRegistryUri uri="silkit://localhost:..."
OnParticipantConnected simulation="" participant="Example"
OnParticipantStatusUpdate simulation="" participant="Example" status=...
OnServiceDiscoveryEvent simulation="" participant="Example" event=... supplemental={...}
OnMetricsUpdate simulation="" origin="Example" metrics=...
OnParticipantDisconnected simulation="" participant="Example"
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
Configured SIL Kit stdout logging may appear alongside listener output.

For simulation names starting with `session/`, the first participant in each
new generation also produces a notification before its connection notification:

```text
OnGenerationCreated simulation="session/example" generation=1
OnParticipantConnected simulation="session/example" participant="Example"
```

Joining an existing generation or initializing its lifecycle state does not
repeat the event. Recreating an emptied generation (including reusing its
number) does. Legacy simulation names do not produce generation notifications.

## Listener behavior and limitations

- All nine callbacks are printed, including generation creation, required participants, discovery
  supplemental data, and the complete metrics update via SIL Kit's formatters.
  The output is diagnostic text, not a stable JSON/wire format.
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
It checks every callback, concurrent generations, two simulation names,
disconnection, signal shutdown with a participant connected, startup errors,
and a TCP port conflict.

```sh
cmake -S registry-listener -B build-registry-listener \
    -DCMAKE_BUILD_TYPE=Debug -DREGISTRY_LISTENER_BUILD_TESTS=ON
cmake --build build-registry-listener --target registry-listener registry-listener-test-participant -j 4
ctest --test-dir build-registry-listener -R registry-listener-smoke --output-on-failure
```
