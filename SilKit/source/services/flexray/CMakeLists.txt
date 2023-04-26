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


add_library(I_SilKit_Services_Flexray INTERFACE)

target_include_directories(I_SilKit_Services_Flexray
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Flexray
    INTERFACE SilKitInterface
)


add_library(O_SilKit_Services_Flexray OBJECT
    FlexrayController.cpp
    FlexrayController.hpp
    FlexrayDatatypeUtils.cpp
    FlexrayDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp

    FlexraySerdes.hpp
    FlexraySerdes.cpp
)

target_link_libraries(O_SilKit_Services_Flexray
    PUBLIC I_SilKit_Services_Flexray

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Services_Logging
    PRIVATE I_SilKit_Tracing
    PRIVATE I_SilKit_Config
)

add_silkit_test(Test_SimFlexrayValidation SOURCES Test_Validation.cpp LIBS S_SilKitImpl)
add_silkit_test(Test_SimFlexrayController
    SOURCES
        Test_FlexrayController.cpp
        Test_FlexrayControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test(Test_SimFlexray_Serdes SOURCES Test_FlexraySerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
