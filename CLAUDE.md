# SIL Kit — Claude Code Context

SIL Kit is a distributed simulation middleware for automotive ECU development. It connects simulation participants (vECUs, network simulators, test tools) over a registry-based peer-to-peer network and provides bus simulation services for CAN, LIN, Ethernet, and FlexRay.

## Repository layout

- `SilKit/include/silkit/` — public C++ and C API headers, an hour-glass that wraps the C-API to stay ABI and API compatible
- `SilKit/source/services/` — per-bus service implementations (can/, lin/, ethernet/, flexray/, pubsub/, rpc/)
- `SilKit/source/wire/` — internal wire message types (serializable versions of public frame types)
- `SilKit/source/core/` — participant, VAsio transport, service discovery, orchestration
- `SilKit/source/capi/` — C API implemens the actual ABI (c functions, datatypes, typedefs etc.) for the sil kit library.
- `docs/` — Sphinx RST documentation; `docs/api/services/` has per-service API references
- `docs/development/design/` — internal design documents (Markdown)

## Stability guarantees
- we use semver: see `SilKit/docs/for-developers/versioning.md`
- we do not break the networking layer. the network layer has it's own protocol versioning, and capability system, which allows staying backward compatible.
- if possible we write unit tests (using gtest and/or gmock)
- then we add integration tests, that consume public APIs and link against the sil-kit target in cmake (we also use gtest for writing `ITest_*`s)
- the C-API is an ABI stable boundary
- the files in `SilKit/include` are an header-only implementation of a C++ API on top of the C-API (referred to as the hourglass).
- in the long run we want to have the `SilKit/source` to live in a flat namespace hierarchy of `VSilkit::`, which it currently does not. The namespaces in `SilKit/include` are part of the API and not to be changed.
- the experimental namespaces / symbols are exepmt from stability

## Coding conventions

- SPDX license header on every new file: `// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH` + `// SPDX-License-Identifier: MIT`, update for current year
- Namespace: `SilKit::Services::<BusName>` for service code
- New bus types follow the CAN/LIN pattern: public datatypes -> C-API -> wire messages → IMsgFor* interfaces → SimBehavior (trivial + detailed) → controller implementation → serdes
- Wire message types must be registered in `SilKit/source/core/internal/traits/SilKitMsgTraits.hpp` and related type traits. Wire message types are not always necessary, sometimes we can transfer the api messages themselves.
- Controller type keys go in `SilKit/source/core/internal/ServiceConfigKeys.hpp`
- for platform support we are limited to C++17, but want to upgrade to C++20 or later in the future
- prefer C++ composition over inheritance. templates and static dispatch are to be preferred
- always use clang-format to format c++, aim for compile-time safety.
- header only declarations should first have the declarations, and then the implementations after the declaration ina block (this is currently not done everywhere)


## Design documents

Internal design documents live in `docs/development/design/`:

- [i2c-bus-api.md](docs/development/design/i2c-bus-api.md) — future I2C bus API and simulation model design (decisions, wire messages, SimBehavior, accuracy constraints, file list)
