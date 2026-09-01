---
orphan: true
---

# !!! Releasing SIL Kit: Version Bumps and the Changelog

This page is for SIL Kit maintainers. It describes how the version number is
stored in the source tree and how to change it.

**Everything on this page is done by one tool: `sil-kit-generate-version`.**
Do not edit the version by hand: several files have to agree, and the tool is
what keeps them in sync.

## Where the version lives

| File | Who writes it |
| --- | --- |
| `SilKit/cmake/SilKitVersion.cmake` | **Source of truth.** Patched by the tool. Everything in the build reads the version from here. |
| `SilKit/include/silkit/capi/SilKitVersionMacros.h` | **Generated. Never edit by hand.** Committed to the repository, and compiled into the library and into the Windows resources of the utilities. |
| `docs/changelog/versions/latest.md` | Hand-written during development, reset by the tool on a bump. |
| `docs/changelog/versions/<version>.md` | Created by the tool when it archives `latest.md`. |
| `docs/changelog/overview.rst` | The toctree line for the archived entry is added by the tool. |

The build number is deliberately absent from that list: it is set at configure
time, not stored in the source tree. See the "Build identity" section below.

```{warning}
`silkit/capi/Version.h` and `silkit/SilKitVersion.hpp` are **hand-written public
API** headers that declare the version *query functions*. They contain no version
numbers and are never touched by a version bump. Only
`silkit/capi/SilKitVersionMacros.h` is generated.

The tool refuses to overwrite any file that is not recognizably a generated
version-macros header, so a mistyped output path fails instead of destroying an
API header.
```

`docs/changelog/versions/template.md` is authoring guidance for writing a
changelog entry. It is excluded from the documentation build and is not copied
anywhere by the tool.

## Build the tool

```
cmake --build <build-dir> --target sil-kit-generate-version
```

The executable lands in `<build-dir>/sil-kit-generate-version` (with
multi-config generators such as Visual Studio: `<build-dir>/<CONFIG>/`).

The tool is standalone: C++17 and the standard library only, no SIL Kit
dependencies, and it reads the git hash out of `.git` without invoking `git`.
It has to be, because it generates a header the library itself is built from.

## Bump the version

Run it from anywhere inside the source tree; it locates the tree root by looking
for `SilKit/cmake/SilKitVersion.cmake`.

Look at the plan first:

```
sil-kit-generate-version --dry-run --major 5 --minor 0 --patch 9
```

Then apply it:

```
sil-kit-generate-version --major 5 --minor 0 --patch 9
```

That single command:

1. sets the version in `SilKit/cmake/SilKitVersion.cmake` to `5.0.9`,
2. renames the current in-progress changelog entry to its release version and
   date, archiving `docs/changelog/versions/latest.md` as
   `docs/changelog/versions/5.0.8.md`,
3. adds `versions/5.0.8.md` to the toctree in `docs/changelog/overview.rst`,
4. resets `latest.md` to an empty `# [5.0.9] - UNRELEASED` stub,
5. regenerates `SilKit/include/silkit/capi/SilKitVersionMacros.h`.

Either all of it happens or none of it does: every precondition is checked before
the first byte is written.

Review the result with `git diff`. Exactly these files should have changed:

```
SilKit/cmake/SilKitVersion.cmake
SilKit/include/silkit/capi/SilKitVersionMacros.h
docs/changelog/overview.rst
docs/changelog/versions/latest.md
docs/changelog/versions/<old-version>.md   (new file)
```

The `SilKitVersion.cmake` diff must be three lines at most. If it is larger,
something went wrong; do not commit it.

The archived entry gets today's date. Pass `--date YYYY-MM-DD` to set a
different release date, and `--suffix rc1` for a pre-release version (which
turns the version string into `5.0.9-rc1`).

The commit convention for a bump is `version: bump to X.Y.Z (#PR)`.

## Refresh the generated header without bumping

After a rebase or a merge, the git hash in the committed header is stale, and if
the rebase pulled in someone else's bump the version numbers may disagree too.
Run the tool with no version arguments:

```
sil-kit-generate-version
```

This takes the version from `SilKitVersion.cmake`, refreshes the git hash, and
touches nothing else. No changelog rotation happens because the version did not
change.

## Verify

```
sil-kit-generate-version --check
```

Exits non-zero if the header disagrees with `SilKitVersion.cmake`, and writes
nothing. Suitable for a CI guard. Reconfiguring the build is the other check:
`SilKit/source/CMakeLists.txt` fails outright if the generated header is missing.

## Finish the changelog before a release

`latest.md` is the entry for the version currently under development. Fill it in
as changes land, following `template.md`. It is what the packaging step ships as
`CHANGELOG.md`, so it must be complete before packaging.

The heading stays `# [<version>] - UNRELEASED` until the bump; the tool then
substitutes the release date automatically when it archives the file. There is no
need to edit the date by hand.

## Build identity

Nothing on this page stamps a build. This repository does not produce releases;
its GitHub workers run CI and component tests only.

What identifies a build:

- **Git hash.** `SILKIT_GIT_HASH` in the generated header, exposed at runtime as
  `SilKit::Version::GitHash()`. Because the header is committed, the hash is
  whatever HEAD was when the tool last ran, so for a bump commit it is that
  commit's parent. It is no longer refreshed on every build, which is what makes
  the build reproducible.
- **Reproducible builds.** `SILKIT_BUILD_REPRODUCIBLE` is `ON` by default and
  omits timestamps and unique build ids, so the same sources produce the same
  binaries.

**Build number.** `SILKIT_BUILD_NUMBER` exists only because
`SilKit_Version_BuildNumber()` is part of the stable public C API. It is a
property of a build, not of the source tree, so it is *not* stored in the
generated header and `sil-kit-generate-version` cannot set it. To stamp one, pass
it to CMake:

```
cmake -B <build-dir> -DSILKIT_BUILD_NUMBER=42
```

It reaches the library and the Windows `FILEVERSION` as a compile definition on
`I_SilKit`; `SilKitVersionMacros.h` only carries the `#ifndef` fallback of `0` for
consumers who compile against the installed header without CMake. Deliberately
setting it defeats reproducibility, so leave it alone unless something downstream
genuinely needs it.

## Troubleshooting

### `error: refusing to overwrite <path>`

The file is not a generated version-macros header. You most likely passed the
wrong output path. Drop the path argument entirely; the default is correct.

### `error: <version>.md already exists`

`SilKitVersion.cmake` names a version whose changelog entry has already been
archived, which usually means the tree is mid-rebase and the version numbers are
not what you think. Check `SilKitVersion.cmake` and `docs/changelog/versions/`
before doing anything else. `--force` overwrites the archived entry;
`--no-changelog` bumps the version and leaves the changelog alone. Nothing has
been written yet at this point.

### `error: version drift` (from `--check`)

The committed header and `SilKitVersion.cmake` disagree. Run
`sil-kit-generate-version` with no version arguments to regenerate the header.

### `error: no SIL Kit source tree found`

The working directory is outside the source tree. `cd` into it or pass
`--source-dir PATH`.

### `warning: could not determine the git hash`

`.git` was not found or could not be read; `SILKIT_GIT_HASH` is written as
`UNKNOWN`. Pass `--git-dir PATH`, or `--git-hash HASH` to set it directly.

`sil-kit-generate-version --help` lists every option.

## See also

- {doc}`build` for build configuration and packaging
- `docs/for-developers/versioning.md` for what the version numbers promise
