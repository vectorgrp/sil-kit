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

    message(STATUS "SIL Kit: disabling warnings for dashboard. Fixme")
    silkit_clean_default_compileflags()
    add_compile_options(-w)

    add_library(I_SilKit_Dashboard INTERFACE)
    target_include_directories(I_SilKit_Dashboard
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Client
            ${CMAKE_CURRENT_SOURCE_DIR}/Dto
            ${CMAKE_CURRENT_SOURCE_DIR}/Service
            ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests
    )

    add_library(S_SilKit_Dashboard STATIC
        Client/CreateSimulationCoroutine.hpp
        Client/DashboardComponents.hpp
        Client/DashboardSystemApiClient.hpp
        Client/DashboardSystemServiceClient.cpp
        Client/DashboardSystemServiceClient.hpp
        Client/DashboardRetryPolicy.cpp
        Client/DashboardRetryPolicy.hpp
        Client/IDashboardSystemServiceClient.hpp
        Client/SendCoroutine.hpp

        Dto/DataPublisherDto.hpp
        Dto/MatchingLabelDto.hpp
        Dto/ParticipantStatusDto.hpp
        Dto/RpcClientDto.hpp
        Dto/ServiceDto.hpp
        Dto/SimulationConfigurationDto.hpp
        Dto/SimulationCreationRequestDto.hpp
        Dto/SimulationCreationResponseDto.hpp
        Dto/SimulationEndDto.hpp
        Dto/SystemStatusDto.hpp

        IntegrationTests/ClientServerTestRunner.hpp
        IntegrationTests/DashboardSystemApiController.hpp
        IntegrationTests/DashboardTestComponents.hpp

        Service/CachedData.hpp
        Service/CachingSilKitEventHandler.cpp
        Service/CachingSilKitEventHandler.hpp
        Service/ISilKitEventHandler.hpp
        Service/ISilKitToOatppMapper.hpp
        Service/SilKitEventHandler.cpp
        Service/SilKitEventHandler.hpp
        Service/SilKitToOatppMapper.cpp
        Service/SilKitToOatppMapper.hpp

        CreateDashboard.hpp
        CreateDashboard.cpp
        Dashboard.cpp
        Dashboard.hpp
        IDashboard.hpp
        OatppContext.cpp
        OatppContext.hpp
        TestResult.hpp
    )
    target_link_libraries(S_SilKit_Dashboard
        PUBLIC
            I_SilKit_Dashboard
            SilKitInterface
            I_SilKit_Core_Internal
            I_SilKit_Util
            I_SilKit_Core_Participant
            I_SilKit_Services_Logging
        PRIVATE
            Threads::Threads
            S_SilKit_Util_Uri
            S_SilKit_Util_SetThreadName
            oatpp
            yaml-cpp
    )

    target_include_directories(S_SilKit_Dashboard
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Client
        ${CMAKE_CURRENT_SOURCE_DIR}/Dto
        ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests
        ${CMAKE_CURRENT_SOURCE_DIR}/Service
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    silkit_target_clean_compileflags(S_SilKit_Dashboard)
    set_property(TARGET S_SilKit_Dashboard PROPERTY FOLDER "Dashboard")

    add_silkit_test(Test_DashboardCachingSilKitEventHandler
        SOURCES Service/Test_DashboardCachingSilKitEventHandler.cpp 
        LIBS
        S_SilKit_Config
        S_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        )

    add_silkit_test(Test_DashboardSilKitEventHandler
        SOURCES Service/Test_DashboardSilKitEventHandler.cpp 
        LIBS
        S_SilKit_Config
        S_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        oatpp
        )

    add_silkit_test(Test_DashboardSilKitToOatppMapper
        SOURCES Service/Test_DashboardSilKitToOatppMapper.cpp 
        LIBS
        S_SilKit_Config
        S_SilKit_Dashboard
        I_SilKit_Core_Internal
        oatpp
        )

    add_silkit_test(Test_DashboardSystemServiceClient
        SOURCES Client/Test_DashboardSystemServiceClient.cpp 
        LIBS
        S_SilKit_Config
        S_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_Participant
        oatpp
        )

else()

    add_library (S_SilKit_Dashboard 
        CreateDashboard.hpp
        DashboardUnavailable.cpp
        IDashboard.hpp
        TestResult.hpp)

    target_include_directories(S_SilKit_Dashboard
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(S_SilKit_Dashboard
        PUBLIC
            SilKitInterface
            I_SilKit_Core_Internal
            I_SilKit_Util
            I_SilKit_Core_Participant
    )

endif()

