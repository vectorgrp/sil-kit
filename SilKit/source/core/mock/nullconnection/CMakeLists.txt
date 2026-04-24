# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Core_Mock_NullConnection OBJECT
    NullConnectionParticipant.cpp
    NullConnectionParticipant.hpp
)

target_link_libraries(O_SilKit_Core_Mock_NullConnection
    PRIVATE I_SilKit
)

if (MSVC)
    target_compile_options(O_SilKit_Core_Mock_NullConnection PRIVATE "/bigobj")
endif (MSVC)
