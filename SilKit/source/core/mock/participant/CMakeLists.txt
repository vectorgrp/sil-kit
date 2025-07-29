# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_Mock_Participant INTERFACE)

target_include_directories(I_SilKit_Core_Mock_Participant
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(I_SilKit_Core_Mock_Participant
    INTERFACE I_SilKit_Services_Logging
    INTERFACE I_SilKit_Services_Orchestration
    INTERFACE I_SilKit_Core_Internal
)
