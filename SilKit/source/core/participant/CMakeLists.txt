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

add_library(SilKitMwParticipant STATIC
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

target_link_libraries(SilKitMwParticipant
    INTERFACE  SilKitInterface
    PUBLIC
        SilKitInternal
        SilKitMwLogging
        SilKitConfiguration
        SilKitUtil
        SilKitSetThreadName
        SilKitUuid
        SilKitMwSync
        SilKitSimCan
        SilKitSimEthernet
        SilKitSimFlexray
        SilKitSimLin
        SilKitSimDataMessage
        SilKitSimRpc
        SilKitTracing
        SilKitMwService
    INTERFACE  SilKitMwService
    PRIVATE
        SilKitVersion
)

target_link_libraries(SilKitMwParticipant PUBLIC SilKitMwVAsio)

target_include_directories(SilKitMwParticipant
    PUBLIC .
)

if (MSVC)
    target_compile_options(SilKitMwParticipant PRIVATE "/bigobj")
endif (MSVC)

add_silkit_test(Test_MwParticipant 
    SOURCES Test_Participant.cpp 
    LIBS SilKitMwMockNullConnection SilKitConfigurationTestUtils
)

add_silkit_test(
    Test_ValidateAndSanitizeConfig
    SOURCES Test_ValidateAndSanitizeConfig.cpp
    LIBS SilKitMwParticipant
)
