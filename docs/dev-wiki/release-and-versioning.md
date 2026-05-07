# Release and Versioning

This page documents the versioning policy, compatibility boundaries, changelog structure, and packaging facts that are relevant to developers working in this repository.

It intentionally does not describe release execution workflow.

## Scope

This page focuses on:

- how SIL Kit versions are classified
- what compatibility promises developers must preserve
- where version information lives in the repository
- how release notes are structured in the tree
- what packaging metadata and package components exist

This page does not cover:

- release ownership
- branch or tag procedure
- publishing steps
- approval or handoff workflow

## Semantic Versioning

SIL Kit uses semantic versioning with the structure `<MAJOR>.<MINOR>.<PATCH>`.

The intended meaning is:

- `MAJOR`: incompatible public API changes
- `MINOR`: compatible public API changes, such as additions or deprecations
- `PATCH`: compatible bug fixes

When classifying a change, the important question is not how large the implementation diff is.
The important question is whether the externally visible compatibility contract changes.

## Compatibility Commitments

There are a few compatibility boundaries that should be treated as hard constraints.

### Non-Experimental Functionality

Non-experimental functionality should not be removed without prior deprecation.

That means:

- removal is not a routine cleanup action
- deprecation must come first
- version impact must be considered before the removal happens

### ABI Stability

Exported symbol ABI compatibility must be preserved.

Practically, this means developers should avoid incompatible changes such as:

- changing the signature of an exported function in an incompatible way
- modifying exported structures used in function signatures in an incompatible way

Even small-looking ABI mistakes can create very difficult runtime failures for downstream users.

### Network Compatibility

Network protocol compatibility is also part of the contract.

If the protocol changes incompatibly, older participants must still be able to detect the incompatibility and refuse the connection safely.

For developers, this means:

- transport or handshake changes are release-relevant changes
- compatibility must be thought about at the protocol boundary, not only at the API boundary

## API Surface Categories

Not all API surfaces in the repository have the same compatibility status.

### `C` API

The `C` API is the main compatibility anchor.

If a change affects the public API contract in a versioning-sensitive way, the `C` API is the first place to assess impact.

### Header-Only `C++` API

The `C++` API in the public headers is a wrapper around the `C` API and follows an hourglass-style design.

It should be treated as public and user-visible, but it does not carry the same stated inter-version compatibility guarantees as the `C` API.

That does not make it safe to change casually.
It means developers should distinguish between:

- strict versioning commitments
- broader user-facing behavior and source-compatibility expectations

### Experimental API

Experimental API is outside the normal compatibility promise.

Important caveats:

- experimental functions or types may be removed without a major version bump
- experimental signatures still should not be changed incompatibly in a way that breaks ABI silently

This means experimental code has more freedom than the stable API, but not unlimited freedom.

## Where Version Information Lives

The main version metadata is configured in:

- `SilKit/cmake/SilKitVersion.cmake`

The important variables there are:

- `SILKIT_VERSION_MAJOR`
- `SILKIT_VERSION_MINOR`
- `SILKIT_VERSION_PATCH`
- `SILKIT_BUILD_NUMBER`
- `SILKIT_VERSION_SUFFIX`

In practice:

- major, minor, and patch define the visible version line
- build number is separate metadata used by builds and packaging
- suffix support exists for annotated version strings

Developers touching version-sensitive behavior should inspect this file before assuming how version strings are assembled.

## Changelog Layout

Release notes are maintained as markdown files under:

- `docs/changelog/versions/`

The important files and conventions are:

- `latest.md`: notes for the current unreleased line
- versioned files such as `<version>.md`: released notes for past versions
- `template.md`: structure template for a version note

The heading convention supports unreleased notes by using `UNRELEASED` in the heading instead of a date.

From a repository-maintenance perspective, this means:

- unreleased changes should accumulate in `latest.md`
- released notes should follow the established markdown structure
- changelog shape is part of the repo contract even though it is not runtime code

## Packaging Facts

Packaging is configured in the root `CMakeLists.txt` through CPack.

### Package Metadata

The configured package metadata includes:

- package name
- package version
- package vendor
- package contact
- generated package file name assembled from version and platform metadata

The resulting package naming uses inputs such as:

- version
- platform
- architecture
- compiler
- build type information

### Package Components

The configured components include:

- `bin`: binaries
- `dev`: headers and development artifacts
- `utils`: utility tools
- `docs`: documentation, when enabled
- `source`: source package content, when enabled

Not all components are always present.
Some depend on build configuration.

### Build Options That Affect Packaging

Two options matter especially for package contents:

- `SILKIT_BUILD_DOCS`
- `SILKIT_INSTALL_SOURCE`

In practical terms:

- enabling docs allows documentation artifacts to be packaged
- enabling source install allows source-tree packaging content to be included

### Packaging-Oriented Preset

`CMakePresets.json` contains a `distrib` preset that is the most packaging-oriented preset currently defined in the repository.

It is useful for understanding which options are considered important for a release-like build configuration, even when no release procedure is being described here.

## Change Impact Guide

When deciding whether a change is patch-, minor-, or major-relevant, classify it by externally visible impact.

### Usually Patch-Level

Changes like these are usually patch-level:

- fixing implementation bugs without changing public API contracts
- performance or robustness improvements that preserve compatibility
- internal refactoring that does not change public behavior in an incompatible way

### Usually Minor-Level

Changes like these are usually minor-level:

- adding new compatible public functionality
- deprecating existing public functionality while keeping it available
- extending behavior in a backward-compatible way

### Usually Major-Level

Changes like these are usually major-level:

- removing stable public functionality
- changing stable public API behavior incompatibly
- breaking compatibility expectations for downstream users in a way they must actively adapt to

### Experimental Changes

Experimental API changes should be classified separately from the stable API promise.

Even when a major bump is not required for experimental removals, developers should still evaluate:

- whether the change is user-visible
- whether the change breaks ABI silently
- whether transport or runtime compatibility is affected

## Developer Heuristics

Before merging a change with versioning implications, it is usually worth asking:

1. Does this affect the stable public API surface?
2. Does this affect exported ABI?
3. Does this affect participant interoperability or transport handshake behavior?
4. Does this require changelog visibility?
5. Does this change what should appear in packaged outputs?

If the answer to any of these is yes, the change belongs in versioning-aware review rather than being treated as a normal internal refactor.

## Related Pages

- [Build and Test](./build-and-test.md)
- [Core Architecture](./core-architecture.md)
- [Utilities and Processes](./utilities-and-processes.md)
- [Developer Wiki Front Page](./README.md)
