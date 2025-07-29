# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

if(SILKIT_BUILD_DASHBOARD)

    find_package(Threads REQUIRED)
    message(STATUS "SIL Kit: building dashboard")

    function(silkit_dashboard_target_compile_flags TARGET_NAME)
        if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
            target_compile_options("${TARGET_NAME}"
                PRIVATE -Wno-gnu-zero-variadic-macro-arguments
                PRIVATE -Wno-pessimizing-move
                PRIVATE -Wno-c++20-extensions
            )
        endif ()
    endfunction()

    add_silkit_test_executable(SilKitDashboardTests)
    if (${SILKIT_BUILD_TESTS})
        silkit_dashboard_target_compile_flags(SilKitDashboardTests)
    endif ()

    add_library(I_SilKit_Dashboard INTERFACE)
    target_include_directories(I_SilKit_Dashboard
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/client
            ${CMAKE_CURRENT_SOURCE_DIR}/dto
            ${CMAKE_CURRENT_SOURCE_DIR}/service
    )

    add_library(O_SilKit_Dashboard STATIC
        client/DashboardComponents.hpp
        client/DashboardSystemApiClient.hpp
        client/DashboardSystemServiceClient.cpp
        client/DashboardSystemServiceClient.hpp
        client/DashboardRetryPolicy.cpp
        client/DashboardRetryPolicy.hpp
        client/IDashboardSystemServiceClient.hpp

        dto/DataSpecDto.hpp
        dto/MatchingLabelDto.hpp
        dto/ParticipantStatusDto.hpp
        dto/RpcSpecDto.hpp
        dto/ServiceDto.hpp
        dto/SimulationConfigurationDto.hpp
        dto/SimulationCreationRequestDto.hpp
        dto/SimulationCreationResponseDto.hpp
        dto/SystemStatusDto.hpp

        service/ISilKitToOatppMapper.hpp
        service/DashboardRestClient.cpp
        service/DashboardRestClient.hpp
        service/SilKitToOatppMapper.cpp
        service/SilKitToOatppMapper.hpp

        LockedQueue.hpp
        SilKitEvent.hpp

        
        DashboardBulkUpdate.hpp
        OatppHeaders.cpp
        OatppHeaders.hpp

        DashboardInstance.cpp
        CreateDashboardInstance.cpp
    )
    target_link_libraries(O_SilKit_Dashboard
        PUBLIC
            I_SilKit_Dashboard
            SilKitInterface
            I_SilKit_Core_Internal
            I_SilKit_Util
            I_SilKit_Core_Participant
            I_SilKit_Services_Logging
        PRIVATE
            S_SilKitImpl
            Threads::Threads
            oatpp
    )

    silkit_target_clean_compileflags(O_SilKit_Dashboard)
    silkit_dashboard_target_compile_flags(O_SilKit_Dashboard)

    set_property(TARGET O_SilKit_Dashboard PROPERTY FOLDER "Dashboard")


    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Test_DashboardSilKitEventQueue.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES service/Test_DashboardRestClient.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        oatpp
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES service/Test_DashboardSilKitToOatppMapper.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        oatpp
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES client/Test_DashboardSystemServiceClient.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        oatpp
        )

else()

    add_library (O_SilKit_Dashboard
        DashboardUnavailable.cpp
    )

    target_include_directories(O_SilKit_Dashboard
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(O_SilKit_Dashboard
        PUBLIC
            SilKitInterface
            I_SilKit_Core_Internal
            I_SilKit_Util
            I_SilKit_Core_Participant
    )

endif()


