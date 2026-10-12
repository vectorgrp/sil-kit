# SIL Kit (FOSS) — Reference Context

Everything after the next section governs changes to SIL Kit itself. **Writing a program against SIL Kit**
instead → read only the next section; your application follows its own project's conventions.

## Building an application against SIL Kit

- Consume the installed package from CMake: `find_package(SilKit REQUIRED CONFIG)`, then link
  `SilKit::SilKit` (plus `Threads::Threads`). The package config is exported to
  `SilKit/{lib,lib64}/cmake/SilKit`.
- The public API works at C++14 or later — use whatever standard your project already uses.
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

- Ask before opening pull requests, committing, or creating branches.
- DCO applies to what lands on `main` via squash-merge. Leave local and feature-branch commits as they are —
  warn about missing trailers at most, and keep history unrewritten. The PR check
  (`SilKit/ci/check_dco_signed.py`, `pull_request` to `main` only) does inspect every commit and wants
  `Signed-off-by:` from the author, plus the committer when they differ; `fixup` messages are exempt.

## Build and test

```sh
git submodule update --init --recursive   # required: spdlog, asio, fmt, googletest, oatpp
cmake --preset debug                      # -> _build/debug, installs to _install/debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

- Single suite: `ctest --preset debug -R Test_CanSerdes --output-on-failure`, or run the binary in
  `_build/debug/<CONFIG>/` with `--gtest_filter=`.
- Docs: `cmake -D SILKIT_BUILD_DOCS=ON -B _build/docs && cmake --build _build/docs --target Doxygen`.
- `SILKIT_WARNINGS_AS_ERRORS` defaults OFF but every preset sets it ON — a bare `cmake ..` build is more
  permissive than CI. No sanitizer option — CI passes raw
  `-DCMAKE_CXX_FLAGS='-fsanitize=address|undefined|thread'` on the `relwithdebinfo` preset.
- With CMake 4.x add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` (vendored deps).

Merge gates: DCO sign-off and license headers (`SilKit/ci/check_licenses.sh`). The clang-format and
clang-tidy jobs report but always exit 0 — `check_formatting.py` never sets its failure flag, and there is
no `.clang-tidy` in the repo.

## Repository layout

- `SilKit/include/silkit/` is the public header-only C++ API, an hourglass over the C-API implemented in
  `SilKit/source/capi/` (the actual ABI). `SilKit/source/wire/` holds serializable versions of public frame
  types.
- `SilKit/source/` compiles into many object libraries linked into the single `SilKit` shared library.
  Sources are explicit lists in the nearest `add_library(O_... OBJECT ...)`; no globbing. Target prefixes:
  `O_*` object libs, `I_*` interface libs, `S_*` aggregates. Public headers under `SilKit/include/` *are*
  globbed, so new ones need a re-configure.
- Some integration tests are built on `Demos/`, so demo changes can affect test behaviour.
- `ThirdParty/` is vendored; make changes in SIL Kit's integration code, and document the reason for any
  exception. `cmake/` holds only a packaging template; CMake helpers are in `SilKit/cmake/`.

## Stability guarantees

- Semver; see `docs/for-developers/versioning.md`.
- The networking layer is never broken; it has its own protocol versioning and capability system.
- The C-API is an ABI-stable boundary; nothing crossing it may change layout or signature. The hourglass
  stays header-only.
- `SilKit/include/` namespaces are public API and stay unchanged. Public headers must compile at **C++14**,
  since `Demos/`, `SilKitMonitor` and `SilKitSystemController` build at C++14 deliberately.
- Experimental namespaces/symbols are exempt.

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

- `Test_*.cpp` — unit tests beside the code in `SilKit/source/**`, independent of the public API.
- `ITest_*.cpp` — integration tests in `SilKit/IntegrationTests/`, depending only on the network simulator
  and public `SilKit.{so,dll}`. The `ITest_Internals_*` family deliberately uses internals and builds into
  `SilKitInternalIntegrationTests`.
- `FTest_*.cpp` — functional/perf tests in `SilKit/IntegrationTests/`. GitHub CI runs `-R '^(I|T)'` and so
  skips them; run them locally.
- `SilKit/IntegrationTests/Hourglass/Test_Hourglass*.cpp` is the documented exception to the unit-test rule:
  it tests the public hourglass against a mocked C-API.
- Integration tests start a registry in-process on an ephemeral port; no separate process needed.

## Definition of done

Applies to all work unless the user has asked for a PoC, experiment, reproducer or quick hack — then see
experiment mode below. Never infer that mode.

- **Tests first:** unit tests (`Test_*`) and, for features, integration tests (`ITest_*`), then the
  implementation, with the tests as guard rails. Add backward-compatibility coverage when the change touches
  the wire protocol or configuration.
- **Documentation for every user-visible surface:**
  - Public API — doxygen comments in the header (Doxygen input is `SilKit/include/silkit` only, so comments
    in `source/` never reach the docs) *and* the matching `docs/api/**` page, including the C-API mirror
    under `docs/api/capi/`.
  - Configuration — the owning `docs/configuration/*-configuration.rst`. A new *root* node also needs the
    YAML outline, the Overview table and the toctree in `docs/configuration/configuration.rst`.
  - Utility CLI — `docs/utilities/utilities.rst` and the man page in `docs/man/`.
  - New `.rst` must join a toctree or be marked `:orphan:`; Sphinx runs `-W --keep-going`.
- **Changelog** — only for changes a user can observe: a new or changed feature, a bug fix, or an
  improvement such as performance. Add it to `docs/changelog/versions/latest.md`, written for a user, under
  `## Added` / `## Fixed` / `## Changed`, matching the neighbouring released file. Internal-only work —
  refactors, test-only changes, CI, tooling, agent instructions — gets no entry.
- **The gates that apply to the change**, plus a warnings-as-errors build. The license check scans only
  `.c* .h* .py CMakeLists.txt .sh`, so a docs-only change is exempt.
- **Docs and samples still build:** CI never builds the docs, and only `docs/code-samples/simple/simple.cpp`
  has a build target — an API rename silently rots every other sample. Check with `-DSILKIT_BUILD_DOCS=ON`.
- **A reviewed feature branch** opened as a pull request — see below.

**Adding a participant-configuration option** — the most error-prone user-visible change. All of these are
required; the `schemaPaths_v1` entry is the one that gets missed:

- Struct field and `operator==` in `ParticipantConfiguration.{hpp,cpp}`; `YamlReader.cpp`; `YamlWriter.cpp`;
  `ParticipantConfiguration.schema.json`.
- A matching path in the separate hardcoded `schemaPaths_v1` set in `SilKit/source/config/YamlValidator.cpp`
  — without it, a schema-valid config is rejected at load.
- Round-trip fixtures `ParticipantConfiguration_Full.{json,yaml}` (kept in sync) and the
  `Test_ParticipantConfiguration` / `Test_YamlParser` / `Test_YamlValidator` suites.
- Every example config still validates — `SilKit/source/config/*.{json,yaml}` and `Demos/**/*.silkit.yaml`.
  Run `python3 SilKit/ci/validate_participant_configs.py` (needs `jsonschema pyyaml`); CI runs it on push to
  `main` but *not* on pull requests, so breakage otherwise surfaces only after merge.

## Experiment mode — PoC, reproducer, quick hack

Active only once the user asks for it, until they say otherwise. Aim for the shortest path to the finding:
skip tests, documentation, changelog, doxygen and review; hardcoding, copy-paste and rough formatting are
fine; write only what the throwaway needs. Basic C/C++ hygiene still holds (C++17, no undefined behaviour,
no leaks that matter), existing tests, CI gates and the public API keep working, and the experiment stays
off `main`.

A reproducer can be anything that shows the problem — bad performance, a race, an unexpected log line or a
wrong value count as much as a failing test. The finding is the deliverable. If the experiment turns into
real work, the definition of done applies from then on, not retroactively.

## Reviewing changes

Review a feature branch from each perspective in turn, not as one undifferentiated read. There is no
CODEOWNERS file; see `.github/pull_request_template.md` for the process checklist. For a docs-, CI- or
tooling-only branch, apply only the user perspective and the applicable gates, and report "nothing to add"
for the other two. Every perspective checks against **Stability guarantees** above.

**Security — distributed systems and deployment.** The threat model is a participant receiving
attacker-controlled bytes, so the top surface is deserialization: every `serdes` reader bounds-checks and
survives a malformed or truncated frame without over-reading, over-allocating or crashing. Network-supplied
lengths and counts are capped before they size an allocation; queue and history sizes stay bounded. Version
and capability negotiation degrades gracefully for older peers. Check what a participant listens on by
default and whether the change widens exposure beyond localhost. Check for secrets and filesystem paths
reaching logs. Concurrency counts — ASAN, UBSAN and TSAN all run in CI.

**C++ veteran with style.** Spend review time on what clang-format cannot see (see **Naming and style**):
ownership and lifetime (smart pointers; lifetime of registered handlers relative to their controller),
const-correctness, avoidable copies, and inheritance or a new interface where composition or static dispatch
would do. Clean under `-Wall -Wextra -pedantic` / `/W4` with warnings as errors.

**User perspective and ease of use.** Read the public API as someone meeting it for the first time: is it
discoverable from the existing headers, do the doxygen comments suffice without reading the implementation,
and does a failure produce an error message that says what to change? Does the feature behave sensibly with
no configuration? Do existing configs and older participants still work? Is the changelog entry written for
a user rather than describing the patch? Are the docs where a user would look?

## Naming and style

- SPDX header on every new file, before `#pragma once`, with the bare `//` separator; year is the creation
  year and stays fixed (substantially reworked files use a range, `2022-2025`):
  ```cpp
  // SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
  //
  // SPDX-License-Identifier: MIT
  ```
- `_member` leading underscore for member variables (not `m_`).
- camelCase for locals, parameters and public struct fields.
- PascalCase for methods, classes, namespaces, enum types and enumerators.
- No prefixes for globals and constants; both camelCase and PascalCase occur, so match the file.
- Deliberate exceptions: free functions `to_string(...)` / `toString(...)`.
- `#pragma once` for header guards.
- Quotes for project headers, angle brackets for stdlib; `"silkit/..."` for public API, bare filenames for
  internal headers. Include ordering is deliberately unenforced (`SortIncludes: false`) — keep existing order.
- Public API doxygen: `/*!` blocks with `\brief`, `\param`, `\ref`, and `//!<` for fields.
- Namespace `SilKit::Services::<BusName>` for service code.
- **Format the code you write, not the files you touch.** clang-format output varies between major versions
  (the check only requires >= 14; run `python3 SilKit/ci/check_formatting.py changes`), so reformatting
  untouched code creates churn and merge conflicts. Formatting is not a merge gate. Where your clang-format
  disagrees with surrounding code, leave the surrounding code alone and match the house style by hand:
  120 columns, 4-space indent, Allman braces except after `namespace`, `PointerAlignment: Left`.

## Rules for new code

- C++17 is the limit for platform support; C++20 is a future goal.
- Prefer composition over inheritance; templates and static dispatch over virtual dispatch, especially in
  performance-critical code.
- Smart pointers for ownership. Aim for compile-time safety.
- `auto` functions with trailing return type.
- Interfaces only where they earn their place — testing and dependency injection count.
- Comments only where the code cannot say it.
- Header-only code declares first, then defines in a block after the declarations.
- New bus types follow the CAN/LIN pattern: public datatypes -> C-API -> wire messages -> `IMsgFor*`
  interfaces -> SimBehavior (trivial + detailed) -> controller implementation -> serdes.
- Wire message types are registered in `SilKit/source/core/internal/traits/SilKitMsgTraits.hpp` and related
  traits. They are not always needed — sometimes the API messages can be transferred directly.
- Controller type keys go in `SilKit/source/core/internal/ServiceConfigKeys.hpp`.

## Migrations in progress

Apply to new and touched code only; mixed style in existing files is expected.

- Trailing return types: ~25% of `SilKit/source`, ~35% of `SilKit/include` converted. Both forms coexist
  within a single class.
- `VSilKit` namespace: done in `services/metrics`, `core/vasio/io`, `config`, `dashboard` (~97 files); the
  rest is still `SilKit::Core` / `SilKit::Services::*`, and `VSilKit` code freely references
  `SilKit::Core::*`. Public namespaces in `SilKit/include` remain as they are.

## Where to look

| Path | Topic |
|---|---|
| `README.rst` | Quick start |
| `docs/development/build.rst` | CMake options, presets, docs build, packaging |
| `docs/for-developers/developers.rst` | Architecture, platform support tiers, running a simulation |
| `docs/for-developers/versioning.md` | Semver and API/ABI/protocol compatibility policy |
| `docs/development/rst-help.rst` | Writing the reStructuredText docs |
| `CONTRIBUTING.md` | External pull requests are not currently accepted |
| `.github/pull_request_template.md` | The de-facto PR checklist |
