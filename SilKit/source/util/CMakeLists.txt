# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Util INTERFACE)
target_include_directories(I_SilKit_Util INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_Library(O_SilKit_Util OBJECT
    ExecutionEnvironment.cpp
)
target_link_libraries(O_SilKit_Util
    PUBLIC I_SilKit_Util
    PRIVATE SilKitInterface
    PRIVATE asio
    PRIVATE fmt::fmt-header-only
)

if (MSVC)
    target_compile_definitions(I_SilKit_Util INTERFACE _WIN32_WINNT=0x0601)
endif()
if(MINGW)
    target_compile_definitions(I_SilKit_Util INTERFACE _WIN32_WINNT=0x0601)
endif()

add_subdirectory(tests)



add_library(I_SilKit_Util_FileHelpers INTERFACE)
target_include_directories(I_SilKit_Util_FileHelpers INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_FileHelpers INTERFACE SilKitInterface)

add_library(O_SilKit_Util_FileHelpers OBJECT
    FileHelpers.hpp
    FileHelpers.cpp
)
target_include_directories(O_SilKit_Util_FileHelpers INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_FileHelpers PRIVATE I_SilKit_Util_FileHelpers)


add_library(I_SilKit_Util_StringHelpers INTERFACE)
target_include_directories(I_SilKit_Util_StringHelpers INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_StringHelpers INTERFACE SilKitInterface)

add_library(O_SilKit_Util_StringHelpers OBJECT
    StringHelpers.hpp
    StringHelpers.cpp
)
target_include_directories(O_SilKit_Util_StringHelpers INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_StringHelpers
    PUBLIC I_SilKit_Util_StringHelpers
    PRIVATE fmt::fmt-header-only
)



add_library(I_SilKit_Util_SetThreadName INTERFACE)
target_include_directories(I_SilKit_Util_SetThreadName INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_SetThreadName INTERFACE SilKitInterface)

add_library(O_SilKit_Util_SetThreadName OBJECT
    SetThreadName.hpp
    SetThreadName.cpp
)
target_include_directories(O_SilKit_Util_SetThreadName INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_SetThreadName PUBLIC I_SilKit_Util_SetThreadName)


add_library(I_SilKit_Util_SignalHandler INTERFACE)
target_include_directories(I_SilKit_Util_SignalHandler INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_SignalHandler INTERFACE SilKitInterface)

add_library(O_SilKit_Util_SignalHandler OBJECT
    SignalHandler.hpp
    SignalHandler.cpp
)
target_include_directories(O_SilKit_Util_SignalHandler INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_SignalHandler PUBLIC I_SilKit_Util_SignalHandler)


add_library(I_SilKit_Util_Uuid INTERFACE)
target_include_directories(I_SilKit_Util_Uuid INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(O_SilKit_Util_Uuid OBJECT
    Uuid.hpp
    Uuid.cpp
)
target_include_directories(O_SilKit_Util_Uuid INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_Uuid PUBLIC I_SilKit_Util_Uuid)


add_library(I_SilKit_Util_Uri INTERFACE)
target_include_directories(I_SilKit_Util_Uri INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_Uri INTERFACE SilKitInterface)

add_library(O_SilKit_Util_Uri OBJECT
    Uri.hpp
    Uri.cpp
)

target_link_libraries(O_SilKit_Util_Uri
    PUBLIC I_SilKit_Util_Uri

    PRIVATE ${SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET}
)
target_include_directories(O_SilKit_Util_Uri INTERFACE ${CMAKE_CURRENT_LIST_DIR})


add_library(I_SilKit_Util_LabelMatching INTERFACE)
target_include_directories(I_SilKit_Util_LabelMatching INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(I_SilKit_Util_LabelMatching INTERFACE SilKitInterface)

add_library(O_SilKit_Util_LabelMatching OBJECT
    LabelMatching.hpp
    LabelMatching.cpp
)
target_include_directories(O_SilKit_Util_LabelMatching INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(O_SilKit_Util_LabelMatching PRIVATE I_SilKit_Util_LabelMatching)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_LabelMatching.cpp 
    LIBS O_SilKit_Util_LabelMatching
)
