# SIL Kit (FOSS) — Reference Context

This file governs changes to SIL Kit itself. **Writing a program against SIL Kit** instead: link `SilKit::SilKit`
via `find_package(SilKit REQUIRED CONFIG)`, start from `docs/code-samples/simple/` and
`docs/for-developers/developers.rst`, follow your own project's conventions, and skip the rest of this file.

## Git

- Ask before opening pull requests, committing, or creating branches.
- DCO (`Signed-off-by:` from author and committer) is checked on PRs to `main` by `SilKit/ci/check_dco_signed.py`;
  `fixup` commits are exempt. Leave feature-branch history unrewritten and at most warn about missing trailers.

## Build and test

```sh
git submodule update --init --recursive
cmake --preset debug && cmake --build --preset debug && ctest --preset debug --output-on-failure
ctest --preset debug -R Test_CanSerdes --output-on-failure    # single suite
```

- Presets set `SILKIT_WARNINGS_AS_ERRORS=ON`; a bare `cmake ..` is more permissive than CI.
- Sanitizers have no option: pass `-DCMAKE_CXX_FLAGS=-fsanitize=...` on the `relwithdebinfo` preset, as CI does.
- CMake 4.x needs `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` for the vendored deps.
- **Merge gates** are DCO and license headers (`SilKit/ci/check_licenses.sh`, scans `.c* .h* .py CMakeLists.txt
  .sh`). The clang-format and clang-tidy jobs only report.

**CTest registers one entry per test source file and runs `--gtest_filter=<file stem>.*`. A suite named
differently from its file runs zero cases and reports PASS** — pass `TESTSUITE_NAME` to
`add_silkit_test_to_executable` when they must differ.

- `Test_*.cpp`: unit tests beside the code, independent of the public API (exception:
  `IntegrationTests/Hourglass/Test_Hourglass*.cpp`, testing the hourglass against a mocked C-API).
- `ITest_*.cpp`: only the network simulator and the public `SilKit` library (`ITest_Internals_*` may use
  internals); they start their own in-process registry. `FTest_*.cpp`: functional/perf, skipped by CI — run locally.

## Code map and stability

- `SilKit/include/silkit/` is the public header-only C++ API, an hourglass over the C-API in `SilKit/source/capi/`
  (the actual ABI). Nothing crossing the C-API changes layout or signature; the hourglass stays header-only.
- `SilKit/include/` namespaces are public API. Public headers compile at **C++14** (`Demos/`, `SilKitMonitor`,
  `SilKitSystemController` build at C++14 deliberately). Experimental symbols are exempt.
- Semver per `docs/for-developers/versioning.md`. The network protocol never breaks; it has its own versioning
  and capability negotiation. `SilKit/source/wire/` holds serializable versions of public frame types.
- Sources are explicit lists in the nearest `add_library(O_... OBJECT ...)`, all linked into one `SilKit` shared
  library. Public headers are globbed, so a new one needs a re-configure.
- Some integration tests are built on `Demos/`; demo changes can break them.
- `ThirdParty/` is vendored: change SIL Kit's integration code instead, and document any exception.

## Definition of done

- **Tests first**: `Test_*`, plus `ITest_*` for features; backward-compatibility coverage when the wire protocol
  or configuration changes.
- **Docs for every user-visible surface**: doxygen in the public header (Doxygen reads only `SilKit/include/silkit`)
  plus the `docs/api/**` page and its C-API mirror; `docs/configuration/*-configuration.rst` for config (a new
  root node also needs outline, Overview table and toctree in `configuration.rst`); `docs/utilities/utilities.rst`
  and `docs/man/` for CLI. New `.rst` joins a toctree or is `:orphan:`.
- **Docs build**: CI never builds docs, and only `docs/code-samples/simple/simple.cpp` compiles — check renames
  with `-DSILKIT_BUILD_DOCS=ON`.
- **Changelog** for user-visible changes only: `docs/changelog/versions/latest.md` under `## Added` / `## Fixed` /
  `## Changed`, written for a user.
- The applicable **merge gates** pass with warnings as errors, and the branch is reviewed as a pull request.

**New participant-configuration option** — every item is required; the `schemaPaths_v1` one gets missed:
field and `operator==` in `ParticipantConfiguration.{hpp,cpp}`; `YamlReader.cpp`; `YamlWriter.cpp`;
`ParticipantConfiguration.schema.json`; the hardcoded `schemaPaths_v1` in `SilKit/source/config/YamlValidator.cpp`
(else valid configs are rejected at load); fixtures `ParticipantConfiguration_Full.{json,yaml}`; suites
`Test_ParticipantConfiguration`, `Test_YamlParser`, `Test_YamlValidator`. Then run
`python3 SilKit/ci/validate_participant_configs.py` — CI runs it only after merge.

**Experiment mode** replaces all of the above once the user asks for a PoC, reproducer or quick hack — never infer
it. Take the shortest path to the finding: no tests, docs or changelog; hardcoding is fine. Keep C++17 without
undefined behaviour, existing tests and the public API working, and the experiment off `main`. A slow path, race
or wrong value is as good a reproducer as a failing test.

## Reviewing

Done when security, C++ and user perspectives each have findings citing `file:line` or "nothing to add";
docs/CI/tooling-only branches get only the user perspective.

- **Security**: peers send attacker-controlled bytes — `serdes` readers bounds-check and survive truncated frames,
  network-supplied lengths are capped before allocating, queues stay bounded, default listen addresses stay no wider.
- **C++**: lifetime of registered handlers relative to their controller; everything in Style and Stability.
- **User**: old configs and older peers keep working; defaults work with no configuration; changelog written for a user.

## Style

- SPDX header on new files, before `#pragma once`, year fixed at creation:
  `// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH`, `//`, `// SPDX-License-Identifier: MIT`.
- `_member` fields; camelCase locals, parameters and public struct fields; PascalCase methods, types, namespaces
  and enumerators; no prefixes on globals and constants.
- Quotes for project headers (`"silkit/..."` public, bare filename internal); keep the existing include order.
  Public doxygen: `/*!` with `\brief`, `\param`; `//!<` for fields.
- **Format only the lines you write.** clang-format versions disagree, so leave surrounding code as it is.
- C++17. Composition and static dispatch over inheritance and virtuals; smart pointers for ownership; interfaces
  only where testing or dependency injection needs them; comments only where the code cannot speak. Header-only
  code: declarations first, definitions in a block after.
- New bus types follow CAN/LIN: public datatypes → C-API → wire messages → `IMsgFor*` → SimBehavior (trivial +
  detailed) → controller → serdes. Register wire types in `SilKit/source/core/internal/traits/SilKitMsgTraits.hpp`
  (when API types cannot be sent directly), controller keys in `SilKit/source/core/internal/ServiceConfigKeys.hpp`.
- Migrations, for new and touched code only: internal namespaces move to `VSilKit` (done in `services/metrics`,
  `core/vasio/io`, `config`, `dashboard`; public API keeps `SilKit::*`); trailing return types `auto f() -> T`.

More: `docs/development/build.rst` (CMake options, presets, packaging), `docs/for-developers/developers.rst`
(architecture), `docs/development/rst-help.rst` (writing docs).
