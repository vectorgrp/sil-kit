# [5.0.0] - 2025-07-29


## Added
* API: added new SilKit_ParticipantConfiguration_ToJson function which exports the complete parsed and validated participant configuration as a JSON string.
* dashboard: add performance related metrics.

## Fixed
* public API: removed harmful noexcept and added a missing virtual destructor.

## Changed
* dashboard: removed legacy v1.0 API.
* build: the internal code now uses C++17
* config: we replaced yaml-cpp with rapidyaml and rewrote the YAML/JSON parsing.
