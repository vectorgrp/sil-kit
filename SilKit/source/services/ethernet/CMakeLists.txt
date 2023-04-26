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


add_library(I_SilKit_Services_Ethernet INTERFACE)

target_include_directories(I_SilKit_Services_Ethernet
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Wire_Ethernet
    INTERFACE I_SilKit_Services_Logging
)


add_library(O_SilKit_Services_Ethernet OBJECT
    EthController.cpp
    EthController.hpp

    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    EthernetSerdes.hpp
    EthernetSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Ethernet
    PUBLIC I_SilKit_Services_Ethernet

    PRIVATE I_SilKit_Core_Internal
    PRIVATE SilKitInterface
)

add_silkit_test(Test_SimEthController
    SOURCES 
        Test_EthControllerTrivialSim.cpp
        Test_EthControllerDetailedSim.cpp
        Test_EthControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test(Test_SimEthernet_Serdes  SOURCES Test_EthernetSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
