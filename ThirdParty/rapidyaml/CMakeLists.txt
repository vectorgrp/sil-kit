add_library(rapidyaml STATIC
    src/rapidyaml.cpp
    include/rapidyaml.hpp
    include/rapidyaml-0.15.2.hpp
)

target_include_directories(rapidyaml
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(rapidyaml
	PUBLIC RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1
)

if(UNIX)
    set_target_properties(rapidyaml
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
        CXX_STANDARD 14
    )
endif()

