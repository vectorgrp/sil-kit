---
orphan: true
---

# !!! Version Bumps and the Changelog

For maintainers.

## Where the version lives

`SilKit/include/silkit/capi/SilKitVersionMacros.h` holds `SILKIT_VERSION_MAJOR`,
`_MINOR` and `_PATCH`. It is the single source of truth: `SilKit/cmake/SilKitVersion.cmake`
reads the numbers from it, and the library and the utilities' Windows resources include it.

## Bump the version

```sh
python3 SilKit/ci/bump_version.py --dry-run 5.0.9
python3 SilKit/ci/bump_version.py 5.0.9
```

This sets the version in the header, archives `latest.md` as `5.0.8.md`, lists it in
`docs/changelog/overview.rst` and resets `latest.md` to an empty `# [5.0.9] - UNRELEASED` stub.
The archived heading keeps the release date already in `latest.md`, or gets today's date if it
still says `UNRELEASED` (`--date YYYY-MM-DD` overrides either). Review with `git diff`,
commit as `version: bump to X.Y.Z`, and open a PR to merge the commit to main.

## Build identity

The git hash, build number and pre-release suffix describe a *build*. The header only
carries fallbacks, CMake supplies the real values:

```sh
cmake -B <build-dir> -DSILKIT_BUILD_GIT_HASH=<hash> -DSILKIT_BUILD_NUMBER=42 -DSILKIT_VERSION_SUFFIX=rc1
```

- `SILKIT_BUILD_GIT_HASH` defaults to `git rev-parse HEAD`, evaluated at configure time.
  Without git it falls back to `UNKNOWN`.
- `SILKIT_BUILD_NUMBER` defaults to `0` and also fills the Windows `FILEVERSION`.
- `SILKIT_VERSION_SUFFIX` marks a pre-release. It changes `SilKit::Version::String()`
  to `5.0.8-rc1` and the CPack archive names.

## See also

- {doc}`build` for build configuration and packaging
- `docs/for-developers/versioning.md` for what the version numbers promise
