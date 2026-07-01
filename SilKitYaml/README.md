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
- Depends only on `rapidyaml`. It does **not** depend on SIL Kit. The host
  project must provide a `rapidyaml` target.
- **Not installed and not exported** by SIL Kit; it is an internal interface
  library whose symbols do not leak into the SIL Kit shared library.

## CMake usage

The host project provides a `rapidyaml` target, then:

```cmake
add_subdirectory(SilKitYaml)
target_link_libraries(my_target PRIVATE SilKitYaml::SilKitYaml)
```

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
