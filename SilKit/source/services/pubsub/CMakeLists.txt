# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_PubSub INTERFACE)

target_include_directories(I_SilKit_Services_PubSub
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_PubSub
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Core_Participant
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Wire_Data
)


add_library(O_SilKit_Services_PubSub OBJECT
    DataMessageDatatypeUtils.hpp
    DataMessageDatatypeUtils.cpp
    DataPublisher.hpp
    DataPublisher.cpp
    DataSubscriber.hpp
    DataSubscriber.cpp

    DataSubscriberInternal.hpp
    DataSubscriberInternal.cpp

    DataSerdes.hpp
    DataSerdes.cpp
)

target_link_libraries(O_SilKit_Services_PubSub
    PUBLIC I_SilKit_Services_PubSub

    PRIVATE SilKitInterface
    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Util
    PRIVATE I_SilKit_Util_LabelMatching
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataPublisher.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriber.cpp
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriberInternal.cpp
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_PubSubMatching.cpp 
    LIBS S_SilKitImpl
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSerdes.cpp
    LIBS S_SilKitImpl
)

