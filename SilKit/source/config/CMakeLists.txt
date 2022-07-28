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

add_library(SilKitConfiguration STATIC
    Configuration.hpp
    Configuration.cpp
    ParticipantConfiguration.hpp
    ParticipantConfiguration.cpp
    Validation.hpp
    Validation.cpp
    YamlParser.hpp
    YamlParser.cpp
    YamlConversion.hpp
    YamlConversion.cpp
    YamlSchema.hpp
    YamlSchema.cpp
    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(SilKitConfiguration
    PRIVATE SilKitInterface
    PRIVATE SilKitUtil
    INTERFACE SilKitUtil
    PRIVATE yaml-cpp
    INTERFACE yaml-cpp
)

target_include_directories(SilKitConfiguration
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_silkit_test(Test_MwCfgValidation 
    SOURCES Test_Validation.cpp 
    LIBS SilKitConfiguration
)

add_silkit_test(Test_ParticipantConfiguration 
    SOURCES Test_ParticipantConfiguration.cpp
    LIBS SilKitMwMockNullConnection
    CONFIGS
        ParticipantConfiguration_Minimal.json
        ParticipantConfiguration_Full.json
        ParticipantConfiguration_Logging_Without_File.json
)

add_silkit_test(Test_MwCfgYamlParser 
    SOURCES Test_YamlParser.cpp 
    LIBS SilKitConfiguration yaml-cpp
)

add_silkit_test(Test_MwCfgYamlValidator 
    SOURCES Test_YamlValidator.cpp 
    LIBS SilKitConfiguration yaml-cpp
)

if(SILKIT_BUILD_TESTS)
    add_library(SilKitConfigurationTestUtils INTERFACE)

    target_include_directories(SilKitConfigurationTestUtils
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    )

    target_link_libraries(SilKitConfigurationTestUtils
        INTERFACE SilKitConfiguration
    )
endif()
