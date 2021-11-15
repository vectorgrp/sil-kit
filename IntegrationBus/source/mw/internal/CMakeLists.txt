# The ComAdapter's connection interfaces are collected in this folder
# for easy consumption from external VIBE projects
add_library(IbInternal INTERFACE)
target_include_directories(IbInternal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

