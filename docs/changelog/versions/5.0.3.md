# [5.0.3] - 2026-02-12

## Fixed

- `sil-kit-registry`: fixed crash whith enabled dashboard

- `sil-kit-registry`: allow configuration of the listen URI, when used as Windows system service

- `cmake`: demo and utility `RPATH`s are now set to match the install hierarchy on all systems

- `cmake`: use `CMAKE_POLICY_VERSION_MINIMUM` since `oatpp` uses an outdated `cmake` version

## Changed

- `sil-kit-registry`: stop force-disabling the dashboard when building Linux packages

## Added

- `docs`: added description of handling of CAN message sizes

- `cmake`: added explicit build options to QNX presets

- `flexray`: add second keyslot parameters to the FlexRay node parameters
