# SIL Kit (FOSS) — Reference Context

## Who this is for

Two different jobs. Most of this file is about the first.

- **Changing SIL Kit itself** — the library, utilities, demos or docs in this repo. Everything below applies.
- **Writing your own program against SIL Kit** — read the next section and stop there. The repository
  layout, definition of done, review section and naming rules below govern contributions to *this* repo;
  they do not govern your application's code, which follows your own project's conventions.

## Building an application against SIL Kit

You need the public API and the user documentation, not this repo's internals.

- Consume the installed package from CMake: `find_package(SilKit REQUIRED CONFIG)`, then link
  `SilKit::SilKit` (plus `Threads::Threads`). The package config is exported to
  `SilKit/{lib,lib64}/cmake/SilKit`.
- The public API is header-only over the stable C-API and works at C++14 or later, so use whatever
  standard your project already uses — you are not bound by this repo's C++17 limit.
- Minimal working example: `docs/code-samples/simple/` (`simple.cpp`, `simple.yaml`, `CMakeLists.txt`),
  walked through in `docs/for-developers/developers.rst`. Richer examples in `Demos/`.
- Start from `silkit/SilKit.hpp` and `silkit/services/all.hpp`; create a participant with
  `SilKit::CreateParticipant`. API reference: `docs/api/`.
- Participant behaviour comes from a YAML or JSON participant configuration — see `docs/configuration/`.
- Running a simulation needs `sil-kit-registry` for participant discovery, and usually
  `sil-kit-system-controller` to name the participants and start the run — see `docs/utilities/utilities.rst`
  and `docs/for-users/users.rst`.
- When it misbehaves: `docs/troubleshooting/`, then `docs/faq/faq.rst`.

## Git and pull requests

- Do not open pull requests, commit, or create branches without permission.
- DCO applies to what lands on `main` via squash-merge. Do not enforce it on local or
  feature-branch commits, and never rewrite history to add trailers — warn at most. The PR check
  (`SilKit/ci/check_dco_signed.py`, `pull_request` to `main` only) does inspect every commit and wants
  `Signed-off-by:` from the author, plus the committer when they differ; `fixup` messages are exempt.

## Build and test

```sh
git submodule update --init --recursive   # required: spdlog, asio, fmt, googletest, oatpp
cmake --preset debug                      # -> _build/debug, installs to _install/debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

- Presets: `debug`, `release`, `relwithdebinfo`, `distrib`, `x86-*`, `vs141-*`, plus QNX/ARM cross presets.
- Single suite: `ctest --preset debug -R Test_CanSerdes --output-on-failure`, or run the binary in
  `_build/debug/<CONFIG>/` with `--gtest_filter=`.
- Format: `python3 SilKit/ci/check_formatting.py changes` (staged + modified). Needs clang-format >= 14.
  Output differs between clang-format major versions, so format only what you wrote — see below.
- Docs: `cmake -D SILKIT_BUILD_DOCS=ON -B _build/docs && cmake --build _build/docs --target Doxygen`.
- `SILKIT_WARNINGS_AS_ERRORS` defaults OFF but every preset sets it ON — a bare `cmake ..` build is more
  permissive than CI. Options defaulting ON: `SILKIT_BUILD_TESTS`, `_UTILITIES`, `_DEMOS`, `_DASHBOARD`.
  OFF: `_DOCS`, `_STATIC`, `_INSTALL_SOURCE`. No sanitizer option — CI passes raw
  `-DCMAKE_CXX_FLAGS='-fsanitize=address|undefined|thread'` on the `relwithdebinfo` preset.
- With CMake 4.x add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` (vendored deps).

Merge gates: DCO sign-off and license headers (`SilKit/ci/check_licenses.sh`). The clang-format and
clang-tidy jobs report problems but always exit 0, so neither blocks a merge — `check_formatting.py`
never sets its failure flag ("Only warn for now"), and there is no `.clang-tidy` in the repo.

## Repository layout

- `SilKit/` — the library: public headers, implementation, tests. CMake helper modules in `SilKit/cmake/`
- `Utilities/` — `SilKitRegistry/`, `SilKitSystemController/`, `SilKitMonitor/` (needs `SILKIT_BUILD_UTILITIES=ON`)
- `Demos/` — `communication/` (Can, Ethernet, Flexray, Lin, PubSub, Rpc), `api/`, `tools/Benchmark/`.
  Some integration tests are built on the demos, so changes here can affect test behaviour
- `docs/` — Sphinx sources and `Doxyfile.in`
- `ThirdParty/` — vendored deps; avoid editing, prefer changes in integration code, document any exception
- `cmake/` — packaging template only (`README.txt.in`)

### SilKit/ public surface

- `SilKit/include/silkit/` — public header-only C++ API; hourglass over the C-API, ABI- and API-compatible
- `SilKit/source/capi/` — C-API implementation (the actual ABI: C functions, datatypes, typedefs)

### SilKit/source/ internal areas

- `util/` — shared helpers; `config/` — configuration loading; `tracing/` — tracing support
- `core/` — participant internals, VAsio transport, service discovery, orchestration
- `services/` — per-bus: can, ethernet, lin, flexray, pubsub, rpc, logging, metrics, orchestration
- `wire/` — internal wire message types (serializable versions of public frame types)
- `extensions/`, `experimental/` (incl. network simulator internals), `dashboard/`

The tree compiles into many object libraries linked into the single `SilKit` shared library — a change in a
small submodule affects that library without introducing a new binary.

## Stability guarantees

- Semver; see `docs/for-developers/versioning.md`
- The networking layer is never broken; it has its own protocol versioning and capability system
- The C-API is an ABI-stable boundary; nothing crossing it may change layout or signature
- `SilKit/include/` namespaces are public API and must not change
- Experimental namespaces/symbols are exempt

## Tests

Wired through `SilKit/cmake/SilKitTest.cmake`. Executables: `SilKitUnitTests`, `SilKitIntegrationTests`,
`SilKitInternalIntegrationTests`, `SilKitFunctionalTests`, `SilKitInternalFunctionalTests`,
`SilKitHourglassTests`, `SilKitDashboardTests`, `SilKitRegistryTests`.

```cmake
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_Foo.cpp LIBS S_SilKitImpl I_SilKit)
```

**CTest registers one entry per test suite, named after the source file basename, and runs
`--gtest_filter=<basename>.*`. If the `TEST`/`TEST_F` suite name does not equal the filename stem, the test
runs zero cases and CTest reports PASS.** Use the `TESTSUITE_NAME` argument when they must differ.

- `Test_*.cpp` — unit tests beside the code in `SilKit/source/**`, no dependency on public APIs.
- `ITest_*.cpp` — integration tests in `SilKit/IntegrationTests/`, depending only on the network simulator
  and public `SilKit.{so,dll}`. The `ITest_Internals_*` family deliberately uses internals and builds into
  `SilKitInternalIntegrationTests`.
- `FTest_*.cpp` — functional/perf tests in `SilKit/IntegrationTests/`. GitHub CI runs `-R '^(I|T)'` and so
  skips them; run them locally.
- `SilKit/IntegrationTests/Hourglass/Test_Hourglass*.cpp` is the documented exception to the unit-test rule:
  it tests the public hourglass against a mocked C-API.
- No registry process needed — integration tests start one in-process on an ephemeral port.

Sources are explicit lists in the nearest `add_library(O_... OBJECT ...)`; no globbing. Target prefixes:
`O_*` object libs, `I_*` interface libs, `S_*` aggregates. Public headers under `SilKit/include/` *are*
globbed, so new ones need a re-configure.

## Definition of done

Applies to work intended to merge. For a PoC, reproducer or quick hack, see experiment mode below.

- **Unit tests** (`Test_*`) and, for features, **integration tests** (`ITest_*`) — written first, then the
  implementation, then used as guard rails. Add backward-compatibility coverage when the change touches the
  wire protocol or configuration.
- **Documentation for every user-visible surface:**
  - Public API — doxygen comments in the header (Doxygen input is `SilKit/include/silkit` only, so comments
    in `source/` never reach the docs) *and* the matching `docs/api/**` page, including the C-API mirror
    under `docs/api/capi/`.
  - Configuration — the owning `docs/configuration/*-configuration.rst`. A new *root* node also needs the
    YAML outline, the Overview table, and the toctree in `docs/configuration/configuration.rst`.
  - Utility CLI — `docs/utilities/utilities.rst` and the man page in `docs/man/`.
  - New `.rst` must join a toctree or be marked `:orphan:`; Sphinx runs `-W --keep-going`.
- **Changelog** — only when a user can observe the change: a new or changed feature, a bug fix, or an
  improvement such as performance. Add it to `docs/changelog/versions/latest.md`, written for a user,
  under `## Added` / `## Fixed` / `## Changed`, matching the neighbouring released file. Internal-only
  work needs no entry — refactors, test-only changes, CI, tooling, agent instructions. Do not invent
  entries for invisible changes; they are noise that makes the changelog worse for users.
- **The gates that apply to the change**, plus a warnings-as-errors build. Not all gates apply to all
  changes: the license check scans only `.c* .h* .py CMakeLists.txt .sh`, so a docs-only change is exempt,
  and clang-format warns rather than blocks.
- **A reviewed feature branch** opened as a pull request — see below.

A new participant-configuration option is the most error-prone user-visible change: struct field and
`operator==` in `ParticipantConfiguration.{hpp,cpp}`, `YamlReader.cpp`, `YamlWriter.cpp`,
`ParticipantConfiguration.schema.json`, **and** a matching path in the separate hardcoded `schemaPaths_v1`
set in `SilKit/source/config/YamlValidator.cpp` — omit that last one and a schema-valid config is rejected
at load. Plus the round-trip fixtures `ParticipantConfiguration_Full.{json,yaml}` (keep in sync) and the
`Test_ParticipantConfiguration` / `Test_YamlParser` / `Test_YamlValidator` suites. Every example config
must stay valid against the schema — `SilKit/source/config/*.{json,yaml}` and `Demos/**/*.silkit.yaml`.
Check locally with `python3 SilKit/ci/validate_participant_configs.py` (needs `jsonschema pyyaml`); CI
runs it on push to `main` but *not* on pull requests, so breakage surfaces only after merge.

CI never builds the docs, and only `docs/code-samples/simple/simple.cpp` has a build target — an API rename
silently rots every other sample. Check with `-DSILKIT_BUILD_DOCS=ON`.

## Experiment mode — PoC, reproducer, quick hack

When the user says they want a PoC, an experiment, a reproducer or a quick hack, work that way until they
say otherwise. Do not infer this mode — unless they have said so, the definition of done applies.

In this mode, do not overthink or over-engineer. No tests, documentation, changelog, doxygen or persona
review. Hardcoding, copy-paste and rough formatting are fine. Do not add abstractions, interfaces,
configuration options or error handling a throwaway does not need.

Basic C/C++ hygiene still applies — C++17, no undefined behaviour, no leaks that matter — and so does not
breaking things for others: leave existing tests, CI gates and the public API alone, and do not merge an
experiment into `main`.

A reproducer does not have to be a failing test. Reproducing bad performance, a race, an unexpected log
line or a wrong value is just as valid — the finding is the deliverable, not the code. If the experiment
turns into real work, the definition of done applies to that work, not retroactively to the experiment.

## Reviewing changes

Review a feature branch from each perspective in turn, not as one undifferentiated read. There is no
CODEOWNERS file; see `.github/pull_request_template.md` for the process checklist. For a docs-, CI- or
tooling-only branch, only the user perspective and the applicable gates apply — skip the other two
rather than manufacturing findings.

**Security — distributed systems and deployment.** The threat model is a participant receiving
attacker-controlled bytes, so the top surface is deserialization: every `serdes` reader must bounds-check
and must not over-read, over-allocate, or crash on a malformed or truncated frame. A network-supplied
length or count must never drive an unbounded allocation; queue and history sizes stay bounded. Version and
capability negotiation must degrade gracefully rather than break older peers. Check what a participant
listens on by default and whether the change widens exposure beyond localhost. Check for secrets and
filesystem paths reaching logs. Concurrency counts — ASAN, UBSAN and TSAN all run in CI.

**C++ veteran with style.** Do not spend review time on formatting — it is not a merge gate and versions
disagree. Review what clang-format cannot see:
ownership and lifetime (smart pointers; lifetime of registered handlers relative to their controller),
const-correctness, avoidable copies, and reaching for inheritance or a new interface where composition or
static dispatch would do. C++17 only — and anything in a public header must still compile at **C++14**,
since `Demos/`, `SilKitMonitor` and `SilKitSystemController` build at C++14 deliberately. Nothing crossing
the C-API changes layout; the hourglass stays header-only. Clean under `-Wall -Wextra -pedantic` / `/W4`
with warnings as errors.

**User perspective and ease of use.** Read the public API as someone meeting it for the first time: is it
discoverable from the existing headers, do the doxygen comments suffice without reading the implementation,
and does a failure produce an error message that says what to change? Does the feature behave sensibly with
no configuration? Do existing configs and older participants still work? Is the changelog entry written for
a user rather than describing the patch? Are the docs where a user would look?

## Naming and style

- SPDX header on every new file, before `#pragma once`, with the bare `//` separator; year is the creation
  year and is not bumped later (substantially reworked files use a range, `2022-2025`):
  ```cpp
  // SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
  //
  // SPDX-License-Identifier: MIT
  ```
- `_member` leading underscore for member variables. Never `m_`.
- camelCase for locals, parameters and public struct fields.
- PascalCase for methods, classes, namespaces, enum types and enumerators.
- No prefixes for globals and constants; both camelCase and PascalCase occur, so match the file.
- Deliberate exceptions: free functions `to_string(...)` / `toString(...)`.
- `#pragma once`, never include guards.
- Quotes for project headers, angle brackets for stdlib; `"silkit/..."` for public API, bare filenames for
  internal headers. Include ordering is deliberately unenforced (`SortIncludes: false`) — do not reorder.
- Public API needs doxygen: `/*!` blocks with `\brief`, `\param`, `\ref`, and `//!<` for fields. Not `///`,
  not `@brief`.
- Format the code you write, not the files you touch. clang-format output varies between major versions
  (the check only requires >= 14), so reformatting untouched code or whole files creates churn and merge
  conflicts for no gain. If your clang-format disagrees with surrounding code, leave the surrounding code
  alone. Match the house style by hand where that is easier: 120 columns, 4-space indent, Allman braces
  except after `namespace`, `PointerAlignment: Left`.
- Namespace `SilKit::Services::<BusName>` for service code.

## Rules for new code

- Test-driven: tests capturing the new behaviour first, then the implementation.
- Prefer composition over inheritance; templates and static dispatch over virtual dispatch.
- Use smart pointers. Aim for compile-time safety.
- Use `auto` functions with trailing return type.
- No gratuitous interfaces — but allow for testing and dependency injection.
- No gratuitous comments.
- Allow static dispatch in performance-critical code.
- Header-only code declares first, then defines in a block after the declarations.
- C++17 is the limit for platform support; C++20 is a future goal.
- New bus types follow the CAN/LIN pattern: public datatypes -> C-API -> wire messages -> `IMsgFor*`
  interfaces -> SimBehavior (trivial + detailed) -> controller implementation -> serdes.
- Wire message types are registered in `SilKit/source/core/internal/traits/SilKitMsgTraits.hpp` and related
  traits. They are not always needed — sometimes the API messages can be transferred directly.
- Controller type keys go in `SilKit/source/core/internal/ServiceConfigKeys.hpp`.

## Migrations in progress

Apply to new and touched code; do not mass-refactor. Mixed style in existing files is expected.

- Trailing return types: ~25% of `SilKit/source`, ~35% of `SilKit/include` converted. Both forms coexist
  within a single class.
- `VSilKit` namespace: done in `services/metrics`, `core/vasio/io`, `config`, `dashboard` (~97 files); the
  rest is still `SilKit::Core` / `SilKit::Services::*`, and `VSilKit` code freely references
  `SilKit::Core::*`. Public namespaces in `SilKit/include` remain as they are.

## Where to look

| Path | Topic |
|---|---|
| `README.rst` | Quick start |
| `docs/development/build.rst` | CMake options, docs build, packaging |
| `docs/for-developers/developers.rst` | Architecture, platform support tiers, running a simulation |
| `docs/for-developers/versioning.md` | Semver and API/ABI/protocol compatibility policy |
| `docs/development/rst-help.rst` | Writing the reStructuredText docs |
| `CONTRIBUTING.md` | External pull requests are not currently accepted |
| `.github/pull_request_template.md` | The de-facto PR checklist |

## Navigating changes

1. Find the public API or executable entry point involved.
2. Trace from the relevant `CMakeLists.txt` into the concrete source directory.
3. Identify the owning area under `SilKit/source/`, `Utilities/`, or `Demos/`.
4. Check nearby tests and existing docs before editing.
