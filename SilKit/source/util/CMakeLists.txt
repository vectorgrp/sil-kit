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


add_subdirectory(tests)
