# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_Mock_NullConnection INTERFACE)

target_include_directories(I_SilKit_Core_Mock_NullConnection
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(I_SilKit_Core_Mock_NullConnection
    INTERFACE SilKitInterface
    INTERFACE I_SilKit_Core_Internal
)

add_library(O_SilKit_Core_Mock_NullConnection OBJECT
    NullConnectionParticipant.cpp
    NullConnectionParticipant.hpp
)

target_link_libraries(O_SilKit_Core_Mock_NullConnection
    PUBLIC I_SilKit_Core_Mock_NullConnection

    PRIVATE I_SilKit_Core_Participant
    PRIVATE I_SilKit_Config
)

if (MSVC)
    target_compile_options(O_SilKit_Core_Mock_NullConnection PRIVATE "/bigobj")
endif (MSVC)
