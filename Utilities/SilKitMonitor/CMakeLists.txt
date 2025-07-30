# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.12)
project("sil-kit-monitor" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(SilKitInstall)
include(SilKitBuildTools)

add_executable(sil-kit-monitor
    PassiveSystemMonitor.cpp
    $<$<CXX_COMPILER_ID:MSVC>: "${CMAKE_CURRENT_SOURCE_DIR}/../utilities.manifest" >
)

# Group this demo project into a folder
set_target_properties(sil-kit-monitor PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     INSTALL_RPATH "$ORIGIN/../lib"
)

target_link_libraries(sil-kit-monitor
    SilKit
    I_SilKit_Util

    O_SilKit_Util_SignalHandler
)

# Set versioning infos on exe
if(MSVC)
    get_target_property(SILKIT_BINARY_DIR SilKit BINARY_DIR)
    get_target_property(SILKIT_SOURCE_DIR SilKit SOURCE_DIR)
    # Include the generated version_macros.hpp in SilKit/source
    target_include_directories(sil-kit-monitor
        PRIVATE ${SILKIT_BINARY_DIR}
        PRIVATE ${SILKIT_SOURCE_DIR}
    )
    target_sources(sil-kit-monitor PRIVATE sil-kit-monitor.rc)
endif()

install(TARGETS sil-kit-monitor
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    COMPONENT utils
)
silkit_split_debugsymbols(sil-kit-monitor)
