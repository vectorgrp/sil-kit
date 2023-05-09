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


add_library(I_SilKit_Core_Participant INTERFACE)

target_include_directories(I_SilKit_Core_Participant
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Core_Participant
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_VAsio
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_Logging
    INTERFACE I_SilKit_Services_Orchestration
    INTERFACE I_SilKit_Services_PubSub
    INTERFACE I_SilKit_Services_Rpc
    INTERFACE I_SilKit_Tracing

    # Due to Participant_impl.hpp being used directly in NullConnectionParticipant and
    # RpcTestUtilities, this interface library must also depend on the headers used by
    # the participant implementation.
    INTERFACE I_SilKit_VersionImpl
)


add_library(O_SilKit_Core_Participant OBJECT
    Participant.hpp
    Participant_impl.hpp
    Participant.cpp
    CreateParticipantInternal.hpp
    CreateParticipantInternal.cpp
    ValidateAndSanitizeConfig.hpp
    ValidateAndSanitizeConfig.cpp
    CreateParticipantT.hpp
    CreateParticipantT.cpp
)

target_link_libraries(O_SilKit_Core_Participant
    PUBLIC I_SilKit_Core_Participant

    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Core_Service
    PRIVATE I_SilKit_Core_RequestReply
    PRIVATE I_SilKit_Util
    PRIVATE I_SilKit_Util_SetThreadName
    PRIVATE I_SilKit_Util_Uuid

    PRIVATE I_SilKit_VersionImpl
)

target_link_libraries(O_SilKit_Core_Participant PUBLIC I_SilKit_Core_VAsio)

if (MSVC)
    target_compile_options(O_SilKit_Core_Participant PRIVATE "/bigobj")
endif (MSVC)

add_silkit_test(Test_MwParticipant
    SOURCES Test_Participant.cpp
    LIBS S_SilKitImpl O_SilKit_Core_Mock_NullConnection I_SilKit_Core_Mock_NullConnection I_SilKit_Config_TestUtils
)

add_silkit_test(
    Test_ValidateAndSanitizeConfig
    SOURCES Test_ValidateAndSanitizeConfig.cpp
    LIBS S_SilKitImpl
)
