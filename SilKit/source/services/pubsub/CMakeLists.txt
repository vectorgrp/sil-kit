# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataPublisher.cpp 
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriber.cpp
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriberInternal.cpp
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_PubSubMatching.cpp 
    LIBS S_SilKitImpl
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSerdes.cpp
    LIBS S_SilKitImpl
)
