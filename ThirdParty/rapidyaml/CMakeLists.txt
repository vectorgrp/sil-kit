add_library(rapidyaml-static STATIC
    rapidyaml.hpp
    rapidyaml-0.9.0.cpp
)

target_include_directories(rapidyaml-static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
    set_target_properties(rapidyaml-static
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()

