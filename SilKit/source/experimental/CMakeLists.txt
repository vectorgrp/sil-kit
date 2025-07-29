# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_subdirectory(netsim)

add_library(I_SilKit_Experimental INTERFACE)

target_include_directories(I_SilKit_Experimental
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE SilKitInterface
)


add_library(O_SilKit_Experimental OBJECT
    participant/ParticipantExtensionsImpl.cpp
    participant/ParticipantExtensionsImpl.hpp
    services/lin/LinControllerExtensionsImpl.cpp
    services/lin/LinControllerExtensionsImpl.hpp
    services/orchestration/TimeSyncServiceExtensionsImpl.cpp
    services/orchestration/TimeSyncServiceExtensionsImpl.hpp
)

target_link_libraries(O_SilKit_Experimental
    PUBLIC I_SilKit_Experimental

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Services_Lin
    PRIVATE I_SilKit_Services_Orchestration
    PRIVATE I_SilKit_Util
    PRIVATE I_SilKit_Services_Logging
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES participant/Test_ParticipantExtensionsImpl.cpp
    LIBS O_SilKit_Core_Mock_NullConnection S_SilKitImpl
)

