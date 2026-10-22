# [5.0.9] - UNRELEASED

## Fixed

- LIN: `SendDynamicResponse` now throws a `SilKit::StateError`, as documented, if the controller was not initialized using `InitDynamic`. Previously, the call was silently processed.
