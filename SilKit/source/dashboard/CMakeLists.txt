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

    add_library(O_SilKit_Dashboard STATIC
        client/DashboardPaths.hpp
        client/DashboardSystemServiceClient.cpp
        client/DashboardSystemServiceClient.hpp
        client/IDashboardSystemServiceClient.hpp

        http/AsioHttpClient.cpp
        http/AsioHttpClient.hpp
        http/FakeHttpServer.hpp
        http/HttpResponseParser.cpp
        http/HttpResponseParser.hpp
        http/HttpRetryPolicy.hpp
        http/IHttpClient.hpp
        http/RetryingHttpClient.cpp
        http/RetryingHttpClient.hpp

        dto/BulkUpdateDto.hpp
        dto/DataSpecDto.hpp
        dto/MatchingLabelDto.hpp
        dto/MetricsDto.hpp
        dto/ParticipantStatusDto.hpp
        dto/RpcSpecDto.hpp
        dto/SimulationConfigurationDto.hpp
        dto/SimulationCreationRequestDto.hpp
        dto/SystemStatusDto.hpp

        json/DashboardJson.cpp
        json/DashboardJson.hpp
        json/DashboardJsonWriter.cpp
        json/DashboardJsonWriter.hpp

        service/IDashboardDtoMapper.hpp
        service/DashboardRestClient.cpp
        service/DashboardRestClient.hpp
        service/DashboardDtoMapper.cpp
        service/DashboardDtoMapper.hpp

        LockedQueue.hpp
        SilKitEvent.hpp

        DashboardBulkUpdate.hpp

        DashboardInstance.cpp
        CreateDashboardInstance.cpp
    )
    target_link_libraries(O_SilKit_Dashboard
        PRIVATE I_SilKit
        PRIVATE S_SilKitImpl
        PRIVATE Threads::Threads
    )

    target_compile_definitions(O_SilKit_Dashboard
        PRIVATE ASIO_STANDALONE
    )

    if (MSVC)
        target_compile_options(O_SilKit_Dashboard PRIVATE "/bigobj")
    endif()

    if(MINGW)
        target_link_libraries(O_SilKit_Dashboard PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
    endif()

    silkit_target_clean_compileflags(O_SilKit_Dashboard)
    silkit_dashboard_target_compile_flags(O_SilKit_Dashboard)

    set_property(TARGET O_SilKit_Dashboard PROPERTY FOLDER "Dashboard")


    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES Test_DashboardSilKitEventQueue.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES service/Test_DashboardShutdown.cpp
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES service/Test_DashboardRestClient.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES service/Test_DashboardDtoMapper.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES json/Test_DashboardJsonWriter.cpp
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES http/Test_HttpResponseParser.cpp
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES http/Test_RetryingHttpClient.cpp
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES http/Test_AsioHttpClient.cpp
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

    add_silkit_test_to_executable(SilKitDashboardTests
        SOURCES client/Test_DashboardSystemServiceClient.cpp 
        LIBS
        S_SilKitImpl
        O_SilKit_Dashboard
        I_SilKit
    )

else()

    add_library (O_SilKit_Dashboard
        DashboardUnavailable.cpp
    )

    target_link_libraries(O_SilKit_Dashboard
        PRIVATE I_SilKit
    )

endif()
