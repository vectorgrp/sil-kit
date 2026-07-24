# [5.0.7] - UNRELEASED

## Changed

- `config`: default format of file logging is set to JSON. Set "Format: Simple" in the sink to get previous behaviour.
- `docs`: added description of the logging "Format".

## Fixed

- `config`: a mapping that is mistyped as a YAML sequence (e.g. a stray leading `- ` turning `HealthCheck` into a list) is now rejected with a clear error instead of being silently accepted.
- `config`: a configuration containing multiple `---`-separated YAML documents is now rejected with a clear error. A single leading `---` remains valid.