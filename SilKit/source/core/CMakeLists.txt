# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_subdirectory(internal)
add_subdirectory(service)
add_subdirectory(requests)
add_subdirectory(participant)
add_subdirectory(vasio)
if(SILKIT_BUILD_TESTS)
    add_subdirectory(mock)
endif()
