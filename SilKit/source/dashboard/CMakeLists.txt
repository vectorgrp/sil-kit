# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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
            ${CMAKE_CURRENT_SOURCE_DIR}/Client
            ${CMAKE_CURRENT_SOURCE_DIR}/Dto
            ${CMAKE_CURRENT_SOURCE_DIR}/Service
            ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests
    )

    add_library(O_SilKit_Dashboard STATIC
        Client/DashboardComponents.hpp
        Client/DashboardSystemApiClient.hpp
        Client/DashboardSystemServiceClient.cpp
        Client/DashboardSystemServiceClient.hpp
        Client/DashboardRetryPolicy.cpp
        Client/DashboardRetryPolicy.hpp
        Client/IDashboardSystemServiceClient.hpp

        Dto/DataPublisherDto.hpp
        Dto/DataSpecDto.hpp
        Dto/DataSubscriberDto.hpp
        Dto/MatchingLabelDto.hpp
        Dto/ParticipantStatusDto.hpp
        Dto/RpcClientDto.hpp
        Dto/RpcServerDto.hpp
        Dto/RpcSpecDto.hpp
        Dto/ServiceDto.hpp
        Dto/SimulationConfigurationDto.hpp
        Dto/SimulationCreationRequestDto.hpp
        Dto/SimulationCreationResponseDto.hpp
        Dto/SimulationEndDto.hpp
        Dto/SystemStatusDto.hpp

        IntegrationTests/ClientServerTestRunner.hpp
        IntegrationTests/DashboardSystemApiController.hpp
        IntegrationTests/DashboardTestComponents.hpp

        Service/CachingSilKitEventHandler.cpp
        Service/CachingSilKitEventHandler.hpp
        Service/ICachingSilKitEventHandler.hpp
        Service/ISilKitEventHandler.hpp
        Service/ISilKitEventQueue.hpp
        Service/ISilKitToOatppMapper.hpp
        Service/SilKitEvent.hpp
        Service/SilKitEventHandler.cpp
        Service/SilKitEventHandler.hpp
        Service/SilKitEventQueue.cpp
        Service/SilKitEventQueue.hpp
        Service/SilKitToOatppMapper.cpp
        Service/SilKitToOatppMapper.hpp

        CreateDashboard.hpp
        CreateDashboard.cpp
        Dashboard.cpp
        Dashboard.hpp
        DashboardBulkUpdate.cpp
        DashboardBulkUpdate.hpp
        DashboardParticipant.cpp
        DashboardParticipant.hpp
        IDashboard.hpp
        OatppContext.cpp
        OatppContext.hpp
        TestResult.hpp
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
            yaml-cpp
    )

    silkit_target_clean_compileflags(O_SilKit_Dashboard)
    silkit_dashboard_target_compile_flags(O_SilKit_Dashboard)

    set_property(TARGET O_SilKit_Dashboard PROPERTY FOLDER "Dashboard")

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Service/Test_DashboardCachingSilKitEventHandler.cpp 
        LIBS
        O_SilKit_Config
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Service/Test_DashboardSilKitEventQueue.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Service/Test_DashboardSilKitEventHandler.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        oatpp
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Service/Test_DashboardSilKitToOatppMapper.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        oatpp
        )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Client/Test_DashboardSystemServiceClient.cpp 
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


