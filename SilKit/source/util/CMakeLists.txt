# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_Library(O_SilKit_Util OBJECT
    ExecutionEnvironment.cpp
)
target_link_libraries(O_SilKit_Util
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_FileHelpers OBJECT
    FileHelpers.hpp
    FileHelpers.cpp
)

target_link_libraries(O_SilKit_Util_FileHelpers
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_StringHelpers OBJECT
    StringHelpers.hpp
    StringHelpers.cpp
)

target_link_libraries(O_SilKit_Util_StringHelpers
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_SetThreadName OBJECT
    SetThreadName.hpp
    SetThreadName.cpp
)

target_link_libraries(O_SilKit_Util_SetThreadName
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_SignalHandler OBJECT
    SignalHandler.hpp
    SignalHandler.cpp
)

target_link_libraries(O_SilKit_Util_SignalHandler
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_Uuid OBJECT
    Uuid.hpp
    Uuid.cpp
)

target_link_libraries(O_SilKit_Util_Uuid
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_Uri OBJECT
    Uri.hpp
    Uri.cpp
)

target_link_libraries(O_SilKit_Util_Uri
    PRIVATE I_SilKit
)


add_library(O_SilKit_Util_LabelMatching OBJECT
    LabelMatching.hpp
    LabelMatching.cpp
)

target_link_libraries(O_SilKit_Util_LabelMatching
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_LabelMatching.cpp 
    LIBS O_SilKit_Util_LabelMatching
)


# Maintainer tool for version bumps. Standalone on purpose: it must not depend
# on I_SilKit, since it generates a header the library itself is built from.
add_library(O_SilKit_Util_GenerateVersion OBJECT
    GenerateVersion.hpp
    GenerateVersion.cpp
)

# Root-relative includes as everywhere else in SilKit/source, so consumers write
# #include "util/GenerateVersion.hpp".
target_include_directories(O_SilKit_Util_GenerateVersion
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

add_executable(sil-kit-generate-version sil-kit-generate-version.cpp)
target_link_libraries(sil-kit-generate-version
    PRIVATE O_SilKit_Util_GenerateVersion
)
set_target_properties(sil-kit-generate-version PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)


add_subdirectory(tests)
