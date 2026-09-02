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
        IDashboardInstance.hpp
        IRestClient.hpp
        CreateDashboardInstance.hpp

        client/DashboardPaths.hpp
        client/DashboardSystemServiceClient.cpp
        client/DashboardSystemServiceClient.hpp
        client/IDashboardSystemServiceClient.hpp

        http/AsioHttpClient.cpp
        http/AsioHttpClient.hpp
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
        EventQueueWorkerThread.cpp
        EventQueueWorkerThread.hpp

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


    # Every dashboard test links the same set, so register them from one list.
    set(SILKIT_DASHBOARD_TEST_SOURCES
        Test_DashboardEventQueueWorker.cpp
        Test_DashboardSilKitEventQueue.cpp
        client/Test_DashboardSystemServiceClient.cpp
        http/Test_AsioHttpClient.cpp
        http/Test_HttpResponseParser.cpp
        http/Test_RetryingHttpClient.cpp
        json/Test_DashboardJsonWriter.cpp
        service/Test_DashboardDtoMapper.cpp
        service/Test_DashboardRestClient.cpp
        service/Test_DashboardShutdown.cpp
    )

    target_sources(SilKitDashboardTests PRIVATE http/FakeHttpServer.hpp Mocks/MockRestClient.hpp)

    foreach(testSource IN LISTS SILKIT_DASHBOARD_TEST_SOURCES)
        add_silkit_test_to_executable(SilKitDashboardTests
            SOURCES ${testSource}
            LIBS
            S_SilKitImpl
            O_SilKit_Dashboard
            I_SilKit
        )
    endforeach()

else()

    add_library (O_SilKit_Dashboard
        DashboardUnavailable.cpp
    )

    target_link_libraries(O_SilKit_Dashboard
        PRIVATE I_SilKit
    )

endif()
