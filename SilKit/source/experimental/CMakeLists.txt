# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_subdirectory(netsim)


add_library(O_SilKit_Experimental OBJECT
    participant/ParticipantExtensionsImpl.cpp
    participant/ParticipantExtensionsImpl.hpp
    services/lin/LinControllerExtensionsImpl.cpp
    services/lin/LinControllerExtensionsImpl.hpp
    services/orchestration/TimeSyncServiceExtensionsImpl.cpp
    services/orchestration/TimeSyncServiceExtensionsImpl.hpp
)

target_link_libraries(O_SilKit_Experimental
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES participant/Test_ParticipantExtensionsImpl.cpp
    LIBS O_SilKit_Core_Mock_NullConnection S_SilKitImpl
)
