<!--
SPDX-FileCopyrightText: 2026 Vector Informatik GmbH

SPDX-License-Identifier: MIT
-->

# SilKitYaml — self-contained YAML (de)serialization

`SilKitYaml` is a small, **header-only** C++17 library wrapping
[rapidyaml](https://github.com/biojppm/rapidyaml) with CRTP-based reader/writer
base classes and `Deserialize`/`Serialize` helper functions. It lets you parse
and emit your own YAML/JSON formats against your own schemata.

It is used **internally by SIL Kit** and is intentionally **self-contained**.

## Properties

- Namespace `SilKitYaml`. Errors are reported as `SilKitYaml::YamlError`.
- Depends only on `rapidyaml`. It does **not** depend on SIL Kit.
- **Not installed and not exported** by SIL Kit; it is an internal interface
  library whose symbols do not leak into the SIL Kit shared library.

## CMake usage

```cmake
add_subdirectory(SilKitYaml)
target_link_libraries(my_target PRIVATE SilKitYaml::SilKitYaml)
```

The `rapidyaml` dependency is resolved as follows:

- If the host already defines a `rapidyaml` target (as SIL Kit does in-tree), it
  is used as-is.
- Otherwise SilKitYaml provisions the vendored rapidyaml itself, locating it at
  `${SILKIT_TOP_DIR}/ThirdParty/rapidyaml`. `SILKIT_TOP_DIR` defaults to the SIL
  Kit tree root (self-located as the parent of this folder).

To override for a different layout, set `SILKIT_TOP_DIR` or the more specific
`SILKITYAML_RAPIDYAML_DIR`, or simply provide your own `rapidyaml` target before
`add_subdirectory(SilKitYaml)`.

## Code

```cpp
#include "SilKitYaml/BasicYamlReader.hpp"
#include "SilKitYaml/BasicYamlWriter.hpp"
#include "SilKitYaml/YamlSerdes.hpp"

struct MyConfig
{
    std::string name;
    int count{};
};

struct MyReader : SilKitYaml::BasicYamlReader<MyReader>
{
    using BasicYamlReader::BasicYamlReader;
    using BasicYamlReader::Read;

    void Read(MyConfig& c)
    {
        ReadKeyValue(c.name, "Name");
        OptionalRead(c.count, "Count");
    }
};

struct MyWriter : SilKitYaml::BasicYamlWriter<MyWriter>
{
    using BasicYamlWriter::BasicYamlWriter;
    using BasicYamlWriter::Write;

    void Write(const MyConfig& c)
    {
        MakeMap();
        WriteKeyValue("Name", c.name);
        WriteKeyValue("Count", c.count);
    }
};

auto cfg = SilKitYaml::Deserialize<MyConfig, MyReader>("Name: foo\nCount: 7\n");
auto yaml = SilKitYaml::Serialize<MyConfig, MyWriter>(cfg);
```
