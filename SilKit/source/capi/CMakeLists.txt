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

add_library(O_SilKit_Capi OBJECT
    CapiCan.cpp
    CapiDataPubSub.cpp
    CapiEthernet.cpp
    CapiExceptions.hpp
    CapiFlexray.cpp
    CapiImpl.hpp
    CapiLin.cpp
    CapiLogger.cpp
    CapiOrchestration.cpp
    CapiParticipant.cpp
    CapiRpc.cpp
    CapiUtils.cpp
    CapiVendor.cpp
    CapiVersion.cpp
    CapiNetworkSimulator.cpp
    TypeConversion.hpp
)

target_link_libraries(O_SilKit_Capi
    PUBLIC SilKitInterface

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Config # For ParticipantConfigurationFromXImpl
    PRIVATE I_SilKit_CreateParticipantImpl;
    PRIVATE I_SilKit_CreateSilKitRegistryImpl;
    PRIVATE I_SilKit_Experimental
    PRIVATE I_SilKit_VersionImpl
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiInterfaces.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiCan.cpp LIBS O_SilKit_Services_Can S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiEthernet.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiFlexray.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiData.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiRpc.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiLogger.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiVersion.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiSilKit.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiParticipantStateHandling.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiTimeSync.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiLin.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiSymbols.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CapiNetSim.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)

