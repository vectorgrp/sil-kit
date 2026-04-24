# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Core_RequestReply OBJECT
    RequestReplyDatatypes.hpp

    RequestReplyService.hpp
    RequestReplyService.cpp

    RequestReplySerdes.hpp
    RequestReplySerdes.cpp
)

target_link_libraries(O_SilKit_Core_RequestReply
    PRIVATE I_SilKit
)

add_subdirectory(procs)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_RequestReplyService.cpp 
    LIBS S_SilKitImpl I_SilKit
)
