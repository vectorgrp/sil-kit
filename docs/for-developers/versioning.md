# Semantic Versioning

Version numbers of SIL Kit have the structure `<MAJOR>.<MINOR>.<PATCH>`, e.g., `4.0.55`.
The structure is reminiscent of [semantic versioning], which also uses these same three components.
For SIL Kit however, the version components had different meanings, largely as a relict from the
project that evolved into SIL Kit in the distant past of 2022.

Going forward, SIL Kit will adopt [semantic versioning].

Therefore, we will increment the

- `MAJOR` version if we make any incompatible changes to the API, e.g.,
  - removing deprecated functionality,
- `MINOR` version if we make any compatible changes to the API, e.g.,
  - adding functionality,
  - deprecating existing functionality,
- `PATCH` version if we fix bugs, but keep the API compatible with the previous version.

The first version of SIL Kit using [semantic versioning] will be `5.0.0`.

## Commitments

We will **never**

- remove non-experimental functionality without deprecating it first for at least _one_ delivery,
- break the ABI of exported symbols, e.g.,
  - by changing the signature of an exported function without changing its name in an incompatible manner,
  - by modifying structures defined in the header files and used in the signature of exported functions in an incompatible manner,
- change the network protocol in an incompatible way such that the previous version **is not** able to
  detect the incompatibility and drop the connection.

## SIL Kit API

When the above definitions use the term API, we refer to the `C` API, which is declared in the `silkit/capi/...` include directory.

### Header-Only C++ API

The `C++` API, declared and defined in the `C++` headers in the `silkit` include directory, is a wrapper around the `C` API, following an hourglass pattern.
It can be understood as a standalone header-only library, which depends on the `C` API.
We currently make no statements about inter-version compatibility for the `C++` API.

### Experimental API

The `C` API contains functions and data types named `SilKit_Experimental_...`.
These functions and data types are not subject to the versioning scheme.

- We will never **modify** function signatures or data types in an incompatible manner.
  This would break the ABI of the modified function and create extremely hard to diagnose issues in user code.
- We may **remove** such functions or data types _without_ the major version increment and _without_ prior deprecation.

<!-- ========================================================================================== -->

[semantic versioning]: https://semver.org/