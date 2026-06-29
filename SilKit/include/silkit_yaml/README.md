<!--
SPDX-FileCopyrightText: 2026 Vector Informatik GmbH

SPDX-License-Identifier: MIT
-->

# SilKit::Yaml — auxiliary YAML parsing for out-of-tree projects

This is a **header-only auxiliary library** that exposes the generic YAML
machinery used internally by SIL Kit's configuration parser, so that projects
living outside the SIL Kit tree can parse **their own** YAML formats against
**their own** schemata. It is a thin wrapper around
[rapidyaml](https://github.com/biojppm/rapidyaml).

## Stability

> **UNSTABLE API/ABI.** Unlike the main SIL Kit API (the `silkit/` headers),
> this library does **not** carry any source- or binary-compatibility guarantee
> across SIL Kit versions. The API lives in the internal `VSilKit` namespace and
> may change at any time. **Pin the exact SIL Kit version** you build against.

The bundled `rapidyaml.hpp` is a specific vendored snapshot; the templates are
written against exactly that version. Parsing errors are reported by throwing
`SilKit::ConfigurationError` (from `silkit/participant/exception.hpp`).

## What's here

| Header | Purpose |
|---|---|
| `BasicYamlReader.hpp` | `VSilKit::BasicYamlReader<Impl>` CRTP base for deserialization |
| `BasicYamlWriter.hpp` | `VSilKit::BasicYamlWriter<Impl>` CRTP base for serialization |
| `YamlSerdes.hpp` | `VSilKit::Deserialize<T,R>` / `Serialize<T,W>` / `SerializeAsJson<T,W>` |
| `YamlParserUtils.hpp` | rapidyaml error callbacks + `MakeConfigurationError` |
| `rapidyaml.hpp` | the bundled rapidyaml single header |
| `RapidyamlImpl.cpp` | the one TU that compiles the rapidyaml implementation |

## Usage

Build SIL Kit with `-DSILKIT_INSTALL_YAML=ON` so these files are installed and
the `SilKit::Yaml` CMake target is exported.

```cmake
find_package(SilKit REQUIRED)
add_executable(my_tool
    main.cpp
    ${SILKIT_YAML_INCLUDE_DIR}/silkit_yaml/RapidyamlImpl.cpp   # compile ryml impl ONCE
)
target_link_libraries(my_tool PRIVATE SilKit::Yaml)
```

`SilKit::Yaml` carries the include paths and the required
`RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1` definition. `RapidyamlImpl.cpp` must be
compiled in **exactly one** target — omit it if you already compile rapidyaml
elsewhere.

```cpp
#include "silkit_yaml/BasicYamlReader.hpp"
#include "silkit_yaml/BasicYamlWriter.hpp"
#include "silkit_yaml/YamlSerdes.hpp"

struct MyConfig
{
    std::string name;
    int count{};
};

struct MyReader : VSilKit::BasicYamlReader<MyReader>
{
    using BasicYamlReader::BasicYamlReader;
    using BasicYamlReader::Read;

    void Read(MyConfig& c)
    {
        ReadKeyValue(c.name, "Name");
        OptionalRead(c.count, "Count");
    }
};

struct MyWriter : VSilKit::BasicYamlWriter<MyWriter>
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

auto cfg = VSilKit::Deserialize<MyConfig, MyReader>("Name: foo\nCount: 7\n");
auto yaml = VSilKit::Serialize<MyConfig, MyWriter>(cfg);
```
