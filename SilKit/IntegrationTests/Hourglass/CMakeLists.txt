
add_library(
    SilKit_Hourglass
    INTERFACE
)

target_include_directories(
    SilKit_Hourglass
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    SilKit_Hourglass
    INTERFACE SilKitInterface
    INTERFACE SilKit
)

target_compile_definitions(
    SilKit_Hourglass
    INTERFACE SILKIT_HOURGLASS=1
)
