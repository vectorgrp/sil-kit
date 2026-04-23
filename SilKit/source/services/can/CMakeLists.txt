# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Services_Can OBJECT
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp
    CanController.cpp
    CanController.hpp
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    CanSerdes.hpp
    CanSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Can
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES 
        Test_CanControllerConfig.cpp
        Test_CanControllerDetailedSim.cpp 
        Test_CanControllerTrivialSim.cpp
    LIBS 
        S_SilKitImpl
        I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CanSerdes.cpp LIBS S_SilKitImpl I_SilKit)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CanStringUtils.cpp LIBS S_SilKitImpl)
