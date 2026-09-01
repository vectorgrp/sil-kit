---
orphan: true
---

# !!! Version Bumps and the Changelog

For maintainers. One tool does all of it: `sil-kit-generate-version`. Do not edit
the version by hand, several files have to agree.

## Where the version lives

| File | Who writes it |
| --- | --- |
| `SilKit/cmake/SilKitVersion.cmake` | **Source of truth.** Patched by the tool. |
| `SilKit/include/silkit/capi/SilKitVersionMacros.h` | **Generated. Never edit by hand.** Committed, and compiled into the library and the utilities' Windows resources. |
| `docs/changelog/versions/latest.md` | Hand-written as changes land; reset by the tool on a bump. |
| `docs/changelog/versions/<version>.md` | Written by the tool when it archives `latest.md`. |
| `docs/changelog/overview.rst` | Toctree line for the archived entry, added by the tool. |

```{warning}
`silkit/capi/Version.h` and `silkit/SilKitVersion.hpp` are hand-written public API
declaring the version *query functions*. They hold no version numbers and a bump
never touches them. Only `SilKitVersionMacros.h` is generated, and the tool
refuses to write to anything that is not already a generated version header.
```

## Build the tool

```
cmake --build <build-dir> --target sil-kit-generate-version
```

It lands in `<build-dir>/` (multi-config generators: `<build-dir>/<CONFIG>/`). It
is standalone C++17 with no SIL Kit dependencies, because it generates a header
the library is built from, and it reads `.git` directly rather than invoking
`git`.

Built by default, never installed. `-DSILKIT_BUILD_GENERATE_VERSION=OFF` skips it
when cross-compiling, where a host tool built for the target is useless. The
version logic keeps its unit tests either way.

## Bump the version

Run from anywhere in the source tree. Preview first:

```
sil-kit-generate-version --dry-run --major 5 --minor 0 --patch 9
sil-kit-generate-version --major 5 --minor 0 --patch 9
```

That one command sets the version in `SilKitVersion.cmake`, archives
`latest.md` as `5.0.8.md` with today's date, lists it in `overview.rst`, resets
`latest.md` to an empty `# [5.0.9] - UNRELEASED` stub, and regenerates the
header. Every precondition is checked before the first byte is written, so it
either all happens or none of it does.

Review with `git diff`; exactly five files change and the `SilKitVersion.cmake`
diff is at most three lines. Anything larger means something went wrong.

`--date YYYY-MM-DD` overrides the release date, `--suffix rc1` makes it a
pre-release. Commit as `version: bump to X.Y.Z (#PR)`.

## Refresh the header without bumping

After a rebase or merge the committed hash is stale, and the version may have
drifted from `SilKitVersion.cmake` too. With no version arguments the tool takes
the version from `SilKitVersion.cmake`, refreshes the hash, and rotates nothing:

```
sil-kit-generate-version
sil-kit-generate-version --check   # non-zero on drift, writes nothing; for CI
```

## Build identity

The version is source tree state. The build number and git hash describe a
*build*, so the header carries only `#ifndef` fallbacks and CMake supplies the
real values:

```
cmake -B <build-dir> -DSILKIT_BUILD_GIT_HASH=$(git rev-parse HEAD) -DSILKIT_BUILD_NUMBER=42
```

Pass `SILKIT_BUILD_GIT_HASH` and `SilKit::Version::GitHash()` reports the commit
actually built. Left unset, it reports the header's fallback, which is the commit
that was HEAD when the tool last ran, i.e. the parent of the bump commit.
`SILKIT_BUILD_NUMBER` defaults to `0` and also feeds the Windows `FILEVERSION`.

Neither is refreshed per build, which is what keeps `SILKIT_BUILD_REPRODUCIBLE`
(`ON` by default) meaningful: the same sources and the same flags give the same
binary.

## Troubleshooting

- **`refusing to overwrite <path>`** - not a generated version header, so you
  passed the wrong output path. Drop the path argument; the default is right.
- **`<version>.md already exists`** - `SilKitVersion.cmake` names a version whose
  entry is already archived, usually a mid-rebase tree. Check the version numbers
  first. `--force` overwrites the archived entry, `--no-changelog` skips
  rotation. Nothing has been written yet.
- **`version drift`** (from `--check`) - header and `SilKitVersion.cmake`
  disagree. Re-run with no version arguments.
- **`no SIL Kit source tree found`** - run inside the tree or pass
  `--source-dir PATH`.
- **`could not determine the git hash`** - `.git` was unreadable, so the fallback
  is written as `UNKNOWN`. Pass `--git-dir PATH` or `--git-hash HASH`.

`sil-kit-generate-version --help` lists every option.

## See also

- {doc}`build` for build configuration and packaging
- `docs/for-developers/versioning.md` for what the version numbers promise
